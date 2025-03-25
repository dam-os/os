#include "process.h"
#include "../drivers/system.h"
#include "../memory/paging.h"
#include "../memory/virt_memory.h"
#include "exception.h"
#include "print.h"

// Assuming 2 proc_t struct arguments, saves current registers into arg 0 (a0),
// and loads registers from arg 1 (a1)
// Assumes mscratch has the current stack pointer from the exception
// kernel_entry
#define SAVE_AND_LOAD_REGISTERS()                                              \
  __asm__ __volatile__("sd ra,    8(a0)\n"                                     \
                       "sd s0,   16(a0)\n"                                     \
                       "sd s1,   24(a0)\n"                                     \
                       "sd s2,   32(a0)\n"                                     \
                       "sd s3,   40(a0)\n"                                     \
                       "sd s4,   48(a0)\n"                                     \
                       "sd s5,   56(a0)\n"                                     \
                       "sd s6,   64(a0)\n"                                     \
                       "sd s7,   72(a0)\n"                                     \
                       "sd s8,   80(a0)\n"                                     \
                       "sd s9,   88(a0)\n"                                     \
                       "sd s10,  96(a0)\n"                                     \
                       "sd s11, 104(a0)\n"                                     \
                       "csrr t0, mscratch\n"                                   \
                       "sd t0,  112(a0)\n"                                     \
                       "csrr t0,   mepc\n"                                     \
                       "addi t0, t0, 4\n"                                      \
                       "sd t0,  120(a0)\n"                                     \
                       "ld ra,    8(a1)\n"                                     \
                       "ld s0,   16(a1)\n"                                     \
                       "ld s1,   24(a1)\n"                                     \
                       "ld s2,   32(a1)\n"                                     \
                       "ld s3,   40(a1)\n"                                     \
                       "ld s4,   48(a1)\n"                                     \
                       "ld s5,   56(a1)\n"                                     \
                       "ld s6,   64(a1)\n"                                     \
                       "ld s7,   72(a1)\n"                                     \
                       "ld s8,   80(a1)\n"                                     \
                       "ld s9,   88(a1)\n"                                     \
                       "ld s10,  96(a1)\n"                                     \
                       "ld s11, 104(a1)\n"                                     \
                       "ld sp,  112(a1)\n"                                     \
                       "ld t0,  120(a1)\n"                                     \
                       "csrw mepc,  t0");

proc_t processes[MAX_PROCCESSES];
proc_t *current_proc = NULL;

/**
 * Switch from current_process to next_process by saving registers to
 * current_process, loading registers from next_processing, and jumping back to
 * user mode to the addr in mepc.
 */
__attribute__((naked)) void switch_process(proc_t *current_process,
                                           proc_t *next_process) {
  SAVE_AND_LOAD_REGISTERS();
  __asm__ __volatile__("mret");
}

extern char __kernel_base[], __free_ram_end[];

proc_t *create_process(void *target_function) {
  proc_t *process = NULL;

  int i;
  for (i = 0; i < MAX_PROCCESSES; i++) {
    if (processes[i].state == PROCESS_EMPTY) {
      process = &processes[i];
      break;
    }
  }

  if (!process)
    PANIC("No empty processes");

  process->state = PROCESS_READY;
  process->pid = i;

  process->reg.sp = (uint64)&process->stack[sizeof(process->stack)];

  process->reg.s0 = 0;
  process->reg.s1 = 0;
  process->reg.s2 = 0;
  process->reg.s3 = 0;
  process->reg.s4 = 0;
  process->reg.s5 = 0;
  process->reg.s6 = 0;
  process->reg.s7 = 0;
  process->reg.s8 = 0;
  process->reg.s9 = 0;
  process->reg.s10 = 0;
  process->reg.s11 = 0;
  process->reg.syscall_entry = (uint64)target_function;

  process->reg.ra = (uint64)exit_proc;

  print("Mapping pages!\r\n");
  uint64 *page_table = (uint64 *)alloc_pages(1);
  uint64 paddr = (uint64)__kernel_base;
  while (paddr < (uint64)__free_ram_end) {
    map_virt_mem(page_table, paddr, paddr);
    paddr += PAGE_SIZE;
  }
  map_virt_mem(page_table, 0x10000000, 0x10000000); // Uart
  for (int i = 0x30000000; i < 0x40000000; i += 0x1000)
    map_virt_mem(page_table, i, i); // PCI

  process->page_table = page_table;
  return process;
}

// Initiate the kernel process as the current one
void init_proc(void) {
  current_proc = create_process(NULL);
  current_proc->pid = 0;
  current_proc->state = PROCESS_RUNNABLE;
}

/**
 * Processes can call yeld to hand over context to a different runnable process
 */
void yield(void) {
  // 1. Iterate through every process after current_process
  // 2. Select first runnable process
  // 3. Set current process to new process
  // 4. switch from old to new process
  proc_t *next = NULL;

  int i;
  for (i = 0; i < MAX_PROCCESSES; i++) {
    proc_t *proc = &processes[(current_proc->pid + i + 1) % MAX_PROCCESSES];
    // Switch to process only if it is runnable, and not the kernel process
    if (proc->state != PROCESS_EMPTY && proc->pid > 0) {
      next = proc;
      break;
    }
  }

  if (next == NULL) {
    // No other processes, switch to kernel process
    next = &processes[0];
  }

  proc_t *curr = current_proc;
  current_proc = next;

  uint64 satp_val = (uint64)8 << 60 | (uint64)0x0 << 44 |
                    ((uint64)next->page_table / PAGE_SIZE);
  __asm__ __volatile__("sfence.vma\n"
                       "csrw satp, %[satp]\n"
                       "sfence.vma\n"
                       :
                       : [satp] "r"(satp_val));

  // Start process if its not runnable, else just switch to it
  if (next->state == PROCESS_READY) {
    next->state = PROCESS_RUNNABLE;
  }
  switch_process(curr, next); // ra becomes end of this
}

/**
 * Exits the active process.
 * Registers and stack don't matter, as they will be reset by next process
 * assigned to this pid.
 */
void exit_proc(void) {
  current_proc->state = PROCESS_EMPTY;
  syscall(0, 0, 0, 8);
}
