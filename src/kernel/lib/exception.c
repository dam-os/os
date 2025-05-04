#include "../drivers/system.h"
#include "common.h"
#include "print.h"
#include "process.h"
#include "../memory/virt_memory.h"

__attribute__((naked)) __attribute__((aligned(8))) void kernel_entry(void) {
  __asm__ __volatile__("csrrw sp, mscratch, sp\n" // Set sp to kernel stack (mscratch written in process.c)
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

                       "csrr a0, mscratch\n"   // get old sp, aka entry sp
                       "sd a0, 8 * 30(sp)\n"   // put original sp as saved sp
                       
                       "mv a0, sp\n"  // set trapframe as argument for handle trap (top of kernel stack)
                       "call handle_trap\n"

                       "csrw mscratch, sp\n" // set mscratch back to kernel stack

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
                       "ld sp,  8 * 30(sp)\n"  // Loads the saved entry sp
                       "mret\n");
}
struct trap_frame {
  uint64_t ra;
  uint64_t gp;
  uint64_t tp;
  uint64_t t0;
  uint64_t t1;
  uint64_t t2;
  uint64_t t3;
  uint64_t t4;
  uint64_t t5;
  uint64_t t6;
  uint64_t a0;
  uint64_t a1;
  uint64_t a2;
  uint64_t a3;
  uint64_t a4;
  uint64_t a5;
  uint64_t a6;
  uint64_t a7;
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
  uint64_t sp;
} __attribute__((packed));

#define READ_CSR(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                      \
    __tmp;                                                                     \
  })

#define WRITE_CSR(reg, value)                                                  \
  do {                                                                         \
    uint64_t __tmp = (value);                                                  \
    __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                    \
  } while (0)

#define MCAUSE_ECALL 8

int syscall(int sysno, void* arg1, void* arg2, void* arg3) {
  int res;

  __asm__ __volatile__("ecall"
                       : "=r"(res)
                       : "r"(sysno), "r"(arg1), "r"(arg2), "r"(arg3)
                       : "memory");
  return res;
}

void handle_syscall(struct trap_frame *f) {
  if (f->a0 == 8) { // a1, a2 and a3 are arguments for the syscall
    yield();
  } else if (f->a0 == 1) {
      uint64_t satp_val = READ_CSR(satp);
      uint64_t real_addr = translate_va_to_pa(f->a1, satp_val);

      cprintf("Printing from shell: %s\n", (char *)(real_addr));
  } else if (f->a0 == 2) {
    exit_proc();
  } else {
    PANIC("unexpected syscall a0=%x\n", f->a0);
  }
}

void handle_trap(struct trap_frame *f) {
  uint64_t mcause = READ_CSR(mcause);
  uint64_t mtval = READ_CSR(mtval);
  uint64_t user_pc = READ_CSR(mepc);
  if (mcause == MCAUSE_ECALL) {
    handle_syscall(f);
    user_pc += 4;
    WRITE_CSR(mepc, user_pc);
  } else {

    PANIC("unexpected trap mcause=%x, mtval=%p, mepc=%p\n", mcause, mtval,
          user_pc);
  }
}
