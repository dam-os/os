#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

#define MAX_PROCCESSES 8

#define PROCESS_EMPTY 0
#define PROCESS_RUNNABLE 1
#define PROCESS_READY 2

struct proc_registers {
  uint64_t ra;  //   8
  uint64_t s0;  //  16
  uint64_t s1;  //  24
  uint64_t s2;  //  32
  uint64_t s3;  //  40
  uint64_t s4;  //  48
  uint64_t s5;  //  56
  uint64_t s6;  //  64
  uint64_t s7;  //  72
  uint64_t s8;  //  80
  uint64_t s9;  //  88
  uint64_t s10; //  96
  uint64_t s11; // 104
  uint64_t sp;  // 112
  uint64_t syscall_entry; //120
};
typedef struct proc_registers proc_registers_t;

struct proc {
  int pid;   // 0
  int state; // 4

  proc_registers_t reg; // 8
  // NOTE: Pointers are 64-bit, but the stack is 8-bit aligned!
  uint8_t stack[4096]; // 128
  uint8_t exception_stack[4096*1000]; // Used to store exception data. Always in mscratch when process is running
  uint64_t *page_table;
};
typedef struct proc proc_t;

void switch_process(proc_t *current_process, proc_t *next_process);
proc_t *create_process(void *target_fuction, int isKernel);
void init_proc(void);
void yield(void);
void exit_proc(void);
void exit_proc_syscall(void);

#endif // !PROCESS_H
