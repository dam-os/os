#include "process.h"
#include "system.h"

proc_t processes[MAX_PROCCESSES];
proc_t *current_proc = NULL;

#define SAVE(reg, var)                                                         \
  do {                                                                         \
    __asm__ volatile("mv %0, " reg : "=r"(var));                               \
  } while (0)

#define RESTORE(reg, var)                                                      \
  do {                                                                         \
    __asm__ volatile("mv " reg ", %0" : "=r"(var));                            \
  } while (0)

__attribute__((naked)) void switch_process(proc_t *current_process,
                                           proc_t *next_process) {

  // 1. Copy current registers into the struct pointed to by a1
  // 2. Copy registers from the struct pointed to by a0 into current registers
  // 3. return and pray
  __asm__ __volatile__("sd ra,    8(a0)\n"
                       "sd s0,   16(a0)\n"
                       "sd s1,   24(a0)\n"
                       "sd s2,   32(a0)\n"
                       "sd s3,   40(a0)\n"
                       "sd s4,   48(a0)\n"
                       "sd s5,   56(a0)\n"
                       "sd s6,   64(a0)\n"
                       "sd s7,   72(a0)\n"
                       "sd s8,   80(a0)\n"
                       "sd s9,   88(a0)\n"
                       "sd s10,  96(a0)\n"
                       "sd s11, 104(a0)\n"
                       "sd sp,  112(a0)\n"

                       "ld ra,    8(a1)\n"
                       "ld s0,   16(a1)\n"
                       "ld s1,   24(a1)\n"
                       "ld s2,   32(a1)\n"
                       "ld s3,   40(a1)\n"
                       "ld s4,   48(a1)\n"
                       "ld s5,   56(a1)\n"
                       "ld s6,   64(a1)\n"
                       "ld s7,   72(a1)\n"
                       "ld s8,   80(a1)\n"
                       "ld s9,   88(a1)\n"
                       "ld s10,  96(a1)\n"
                       "ld s11, 104(a1)\n"
                       "ld sp,  112(a1)\n"

                       "ret");
}

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

  process->state = PROCESS_RUNNABLE;
  process->pid = i;

  process->reg.sp = (uint64_t)&process->stack[sizeof(process->stack)];

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

  process->reg.ra = (uint64_t)target_function;

  return process;
}

// Initiate the idle process as the current one
void init_proc(void) {
  current_proc = create_process(NULL);
  current_proc->pid = 0;
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
    // Switch to process only if it is runnable, and not the idle process
    if (proc->state == PROCESS_RUNNABLE && proc->pid > 0) {
      next = proc;
      break;
    }
  }

  if (next == NULL) {
    // @TODO: Don't panic if there's only 1 process, just keep running it!
    PANIC("Failed to yield, no other process found!");
  }

  proc_t *curr = current_proc;
  current_proc = next;

  switch_process(curr, next);
}
