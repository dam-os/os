#include "process.h"
#include "system.h"
#include "paging.h"
#include "virt_memory.h"

// Assuming 2 proc_t struct arguments, saves current registers into arg 0 (a0),
// and loads registers from arg 1 (a1)
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
                       "sd sp,  112(a0)\n"                                     \
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
                       "ld sp,  112(a1)")

proc_t processes[MAX_PROCCESSES];
proc_t *current_proc = NULL;

/**
 * Switch from current_process to next_process by saving registers to
 * current_process, loading registers from next_processing, and simulating a
 * return from the yield() call using ret.
 */
__attribute__((naked)) void switch_process(proc_t *current_process,
                                           proc_t *next_process) {
  SAVE_AND_LOAD_REGISTERS();
  __asm__ __volatile__("ret");
}

/**
 * Start next_process and switch from current_process to it.
 * We can't ret into the start of a function, so we have to start it as a normal
function would be called:
 * 1. Load the next position into ra.
 * 2. Jump to callee.
 * In this case, the "next instruction" is simply the exit_proc function, which
 * handles process exit. The process will "ret" to exit_proc all on its own when
 * it's done.
 */
__attribute__((naked)) void start_switch_process(proc_t *current_process,
                                                 proc_t *next_process) {
  SAVE_AND_LOAD_REGISTERS();
  __asm__ __volatile__("lla ra, %0" ::"i"(exit_proc));
  __asm__ __volatile__("ld t0, 8(a1)\n"
                       "jalr x0, 0(t0)");
}

#define MSTATUS_MPIE 0//(1 << 7)

__attribute__((naked)) void switch_to_umode(void) {
  __asm__ __volatile__(
      "csrw mepc, s0             \n"
      "csrw mstatus, %[mstatus]  \n"
      "mret                      \n"
      :
      : [mstatus] "r" (MSTATUS_MPIE)
  );
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

  process->reg.sp = (uint64_t)&process->stack[sizeof(process->stack)];

  process->reg.s0 = (uint64_t)switch_to_umode;
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

  process->reg.ra = (uint64_t)target_function;

  print("Mapping pages!\r\n");
  uint64_t* page_table = (uint64_t*)alloc_pages(1);
  uint64_t paddr = (uint64_t) __kernel_base;
  while (paddr < (uint64_t) __free_ram_end) {
    map_virt_mem(page_table, paddr, paddr);
    paddr += PAGE_SIZE;
  }

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

/*   __asm__ __volatile__(
    "sfence.vma\n"
    "csrw satp, %[satp]\n"
    "sfence.vma\n"
    :
    : [satp] "r" (0) 
  ); */

  // Start process if its not runnable, else just switch to it
  if (next->state == PROCESS_READY) {
    next->state = PROCESS_RUNNABLE;
    start_switch_process(curr, next);
  } else {

    switch_process(curr, next);
  }
}

/**
 * Exits the active process.
 * Registers and stack don't matter, as they will be reset by next process
 * assigned to this pid.
 */
void exit_proc(void) {
  current_proc->state = PROCESS_EMPTY;
  yield();
}
