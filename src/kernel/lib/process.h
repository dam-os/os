#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

#define MAX_PROCCESSES 8

#define PROCESS_EMPTY 0
#define PROCESS_RUNNABLE 1
#define PROCESS_READY 2

struct proc_registers {
  uint64 ra;            //   8
  uint64 s0;            //  16
  uint64 s1;            //  24
  uint64 s2;            //  32
  uint64 s3;            //  40
  uint64 s4;            //  48
  uint64 s5;            //  56
  uint64 s6;            //  64
  uint64 s7;            //  72
  uint64 s8;            //  80
  uint64 s9;            //  88
  uint64 s10;           //  96
  uint64 s11;           // 104
  uint64 sp;            // 112
  uint64 syscall_entry; // 120
};
typedef struct proc_registers proc_registers_t;

struct proc {
  int pid;   // 0
  int state; // 4

  proc_registers_t reg; // 8
  // NOTE: Pointers are 64-bit, but the stack is 8-bit aligned!
  uint8 stack[4096]; // 120
  uint64 *page_table;
};
typedef struct proc proc_t;

void switch_process(proc_t *current_process, proc_t *next_process);
proc_t *create_process(void *target_fuction);
void init_proc(void);
void yield(void);
void exit_proc(void);

#endif // !PROCESS_H
