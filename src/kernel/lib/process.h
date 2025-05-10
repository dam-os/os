#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

#define MAX_PROCCESSES 8

#define PROCESS_EMPTY 0
#define PROCESS_RUNNABLE 1
#define PROCESS_READY 2

struct proc_registers {
  u64 ra;            //   8
  u64 s0;            //  16
  u64 s1;            //  24
  u64 s2;            //  32
  u64 s3;            //  40
  u64 s4;            //  48
  u64 s5;            //  56
  u64 s6;            //  64
  u64 s7;            //  72
  u64 s8;            //  80
  u64 s9;            //  88
  u64 s10;           //  96
  u64 s11;           // 104
  u64 sp;            // 112
  u64 syscall_entry; // 120
};
typedef struct proc_registers proc_registers_t;

struct proc {
  u32 pid;   // 0
  u32 state; // 4

  proc_registers_t reg; // 8
  // NOTE: Pointers are 64-bit, but the stack is 8-bit aligned!
  u8 stack[4096];           // 128
  u8 exception_stack[4096]; // Used to store exception data. Always in mscratch
                            // when process is running
  u64 *page_table;
};
typedef struct proc proc_t;

void switch_process(proc_t *current_process, proc_t *next_process);
proc_t *create_process(void *target_fuction, u32 isKernel);
void init_proc(void);
void yield(void);
void exit_proc(void);
void exit_proc_syscall(void);

#endif // !PROCESS_H
