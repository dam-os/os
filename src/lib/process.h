#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

#define MAX_PROCCESSES 8

#define PROCESS_EMPTY 0
#define PROCESS_RUNNABLE 1

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
};
typedef struct proc_registers proc_registers_t;

struct proc {
  int pid;   // 0
  int state; // 4

  proc_registers_t reg; // 8
};
typedef struct proc proc_t;

void switch_process(struct proc *current_process, struct proc *next_process);
struct proc *create_process(void *target_fuction);

#endif // !PROCESS_H
