#include "../drivers/system.h"
#include "../drivers/vga.h"
#include "../memory/virt_memory.h"
#include "common.h"
#include "io.h"
#include "process.h"
#include "timer.h"

__attribute__((naked)) __attribute__((aligned(8))) void kernel_entry(void) {
  __asm__ __volatile__(
      "csrrw sp, mscratch, sp\n" // Set sp to kernel stack (mscratch written in
                                 // process.c)
      "addi sp, sp, -8 * 31\n"
      "sd ra,  8 * 0(sp)\n"
      "sd gp,  8 * 1(sp)\n"
      "sd tp,  8 * 2(sp)\n"
      "sd t0,  8 * 3(sp)\n"
      "sd t1,  8 * 4(sp)\n"
      "sd t2,  8 * 5(sp)\n"
      "sd t3,  8 * 6(sp)\n"
      "sd t4,  8 * 7(sp)\n"
      "sd t5,  8 * 8(sp)\n"
      "sd t6,  8 * 9(sp)\n"
      "sd a0,  8 * 10(sp)\n"
      "sd a1,  8 * 11(sp)\n"
      "sd a2,  8 * 12(sp)\n"
      "sd a3,  8 * 13(sp)\n"
      "sd a4,  8 * 14(sp)\n"
      "sd a5,  8 * 15(sp)\n"
      "sd a6,  8 * 16(sp)\n"
      "sd a7,  8 * 17(sp)\n"
      "sd s0,  8 * 18(sp)\n"
      "sd s1,  8 * 19(sp)\n"
      "sd s2,  8 * 20(sp)\n"
      "sd s3,  8 * 21(sp)\n"
      "sd s4,  8 * 22(sp)\n"
      "sd s5,  8 * 23(sp)\n"
      "sd s6,  8 * 24(sp)\n"
      "sd s7,  8 * 25(sp)\n"
      "sd s8,  8 * 26(sp)\n"
      "sd s9,  8 * 27(sp)\n"
      "sd s10, 8 * 28(sp)\n"
      "sd s11, 8 * 29(sp)\n"

      "csrr a0, mscratch\n" // get old sp, aka entry sp
      "sd a0, 8 * 30(sp)\n" // put original sp as saved sp

      "mv a0, sp\n" // set trapframe as argument for handle trap (top of kernel
                    // stack)
      "call handle_trap\n"

      "mv t0, sp\n"           // Get top of kernel sp
      "addi t0, t0, 8 * 31\n" // remote trap args from kernel sp
      "csrw mscratch, t0\n"   // set mscratch back to kernel stack

      "ld ra,  8 * 0(sp)\n"
      "ld gp,  8 * 1(sp)\n"
      "ld tp,  8 * 2(sp)\n"
      "ld t0,  8 * 3(sp)\n"
      "ld t1,  8 * 4(sp)\n"
      "ld t2,  8 * 5(sp)\n"
      "ld t3,  8 * 6(sp)\n"
      "ld t4,  8 * 7(sp)\n"
      "ld t5,  8 * 8(sp)\n"
      "ld t6,  8 * 9(sp)\n"
      "ld a0,  8 * 10(sp)\n"
      "ld a1,  8 * 11(sp)\n"
      "ld a2,  8 * 12(sp)\n"
      "ld a3,  8 * 13(sp)\n"
      "ld a4,  8 * 14(sp)\n"
      "ld a5,  8 * 15(sp)\n"
      "ld a6,  8 * 16(sp)\n"
      "ld a7,  8 * 17(sp)\n"
      "ld s0,  8 * 18(sp)\n"
      "ld s1,  8 * 19(sp)\n"
      "ld s2,  8 * 20(sp)\n"
      "ld s3,  8 * 21(sp)\n"
      "ld s4,  8 * 22(sp)\n"
      "ld s5,  8 * 23(sp)\n"
      "ld s6,  8 * 24(sp)\n"
      "ld s7,  8 * 25(sp)\n"
      "ld s8,  8 * 26(sp)\n"
      "ld s9,  8 * 27(sp)\n"
      "ld s10, 8 * 28(sp)\n"
      "ld s11, 8 * 29(sp)\n"
      "ld sp,  8 * 30(sp)\n" // Loads the saved entry sp
      "mret\n");
}
struct trap_frame {
  u64 ra;
  u64 gp;
  u64 tp;
  u64 t0;
  u64 t1;
  u64 t2;
  u64 t3;
  u64 t4;
  u64 t5;
  u64 t6;
  u64 a0;
  u64 a1;
  u64 a2;
  u64 a3;
  u64 a4;
  u64 a5;
  u64 a6;
  u64 a7;
  u64 s0;
  u64 s1;
  u64 s2;
  u64 s3;
  u64 s4;
  u64 s5;
  u64 s6;
  u64 s7;
  u64 s8;
  u64 s9;
  u64 s10;
  u64 s11;
  u64 sp;
} __attribute__((packed));

#define READ_CSR(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                      \
    __tmp;                                                                     \
  })

#define WRITE_CSR(reg, value)                                                  \
  do {                                                                         \
    u64 __tmp = (value);                                                       \
    __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                    \
  } while (0)

#define MCAUSE_ECALL 8

u32 syscall(u32 sysno, void *arg1, void *arg2, void *arg3) {
  u32 res;

  __asm__ __volatile__("ecall"
                       : "=r"(res)
                       : "r"(sysno), "r"(arg1), "r"(arg2), "r"(arg3)
                       : "memory");
  return res;
}

#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_GETCHAR 3
#define SYS_PUTCHAR 4
#define SYS_SLEEP 5
#define SYS_POWEROFF 6
#define SYS_YIELD 8
#define SYS_EXIT 9

void handle_syscall(struct trap_frame *f) {
  switch (f->a0) {
  case SYS_YIELD:
    yield();
    break;
  case SYS_WRITE: {
    u64 satp_val = READ_CSR(satp);
    u64 real_addr = translate_va_to_pa(f->a1, satp_val);
    cprintf("%s\r\n", (char *)(real_addr));
  } break;
  case SYS_READ: {
  } break;
  case SYS_GETCHAR: {
    char x = cgetchar();
    f->a5 = x;
    break;
  }
  case SYS_PUTCHAR:
    cprintf("CHAR: %x", (char)(u64)f->a1);
    cputchar((char)(u64)(void *)f->a2);
    break;
  case SYS_SLEEP:
    sleep(f->a1);
    break;
  case SYS_POWEROFF:
    poweroff();
    break;
  case SYS_EXIT:
    exit_proc();
    break;
  default:
    PANIC("unexpected syscall a0=%x\r\n", f->a0);
  }
}

void handle_trap(struct trap_frame *f) {
  u64 mcause = READ_CSR(mcause);
  u64 mtval = READ_CSR(mtval);
  u64 user_pc = READ_CSR(mepc);
  if (mcause == MCAUSE_ECALL) {
    handle_syscall(f);
    user_pc += 4;
    WRITE_CSR(mepc, user_pc);
  } else {

    PANIC("unexpected trap mcause=%x, mtval=%p, mepc=%p\r\n", mcause, mtval,
          user_pc);
  }
}
