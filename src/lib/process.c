#include "process.h"
#include "system.h"

struct proc processes[MAX_PROCCESSES];

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

                       "ret");
}

struct proc *create_process(void *target_function) {
  struct proc *process = NULL;

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
  process->pid = i + 1;

  process->reg.s0 = 1;
  process->reg.s1 = 2;
  process->reg.s2 = 3;
  process->reg.s3 = 4;
  process->reg.s4 = 5;
  process->reg.s5 = 6;
  process->reg.s6 = 7;
  process->reg.s7 = 8;
  process->reg.s8 = 9;
  process->reg.s9 = 10;
  process->reg.s10 = 11;
  process->reg.s11 = 12;

  process->reg.ra = (uint64_t)target_function;
  return process;
}
