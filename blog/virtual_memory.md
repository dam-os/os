# Making virtual memory

https://www.simonsungm.cool/2019/10/20/RISC-V-Page-Table-I/

sv-32
https://operating-system-in-1000-lines.vercel.app/en/11-page-table



sv-39
https://pdos.csail.mit.edu/6.828/2021/slides/6s081-lec-vm.pdf


https://marz.utk.edu/my-courses/cosc562c/mmu/

https://chenweixiang.github.io/docs/riscv-privileged-v1.10.pdf#section.4.4


## General info
https://docs.keystone-enclave.org/en/latest/Getting-Started/How-Keystone-Works/RISC-V-Background.html
Could be that M-mode operates exclusively on physical addresses in qemu as well

https://danielmangum.com/posts/risc-v-bytes-privilege-levels/

### S-mode
https://blog.stephenmarz.com/2020/11/23/back-that-s-up/


# Problem: stuck on the mret instruction
gdb just got stuck at the mret instruction and with ctrl+c i could see the $pc was 0.
Solution: disable pmp memory protection in qemu 

# Problem: Instruction Page Fault with valid page table (user mode)
qemu shows valid table, with mapped addrs, but mret still gets Instruction Page Fault. This was because the pages were missing the PAGE_USER flag.

# Debug the page table
Let's lookup the addr 0x080000104
vpn2 = 0x080000104 >> 30 = 0x2
vpn1 = (0x080000104 >> 21) & 0x1ff = 0
vpn0 = (0x080000104 >> 12) & 0x1ff = 0
offset = 0x080000104 & 0x1ff = 0x104
```
(remote) gef➤  x/1x 0x80013000+2*8 // 0x80013000 is the satp start addr, index 2 into vpn2 *8 since each entry is 8 bytes (64 bit)
0x80013010:     0x20005c01         // We get valid page since it ends in 1, we can remove the flags: hex((0x20005c01 >> 10) * 4096) = '0x80017000'
(remote) gef➤  x/1x 0x80017000+0*8 // Now we do the same in vpn1, and get another valid page to vpn0
0x80017000:     0x20006c01
(remote) gef➤  x/1x 0x8001b000+0*8 // lastly we look in vpn0 with offset 0 and get a page ending with 0b1111 which means it is valid, read, write and execute
0x8001b000:     0x2000000f         // Finally we can get the address hex((0x2000000f >> 10) * 4096) = '0x80000000'
```
With 0x80000000 we simply have to add the offset and we get


We can also see the page table in the qemu console:
```
(qemu) info mem
vaddr            paddr            size             attr
---------------- ---------------- ---------------- -------
0000000080000000 0000000080000000 0000000000200000 rwx----
0000000080200000 0000000080200000 0000000000200000 rwx----
0000000080400000 0000000080400000 0000000000200000 rwx----
0000000080600000 0000000080600000 0000000000200000 rwx----
0000000080800000 0000000080800000 0000000000200000 rwx----
0000000080a00000 0000000080a00000 0000000000200000 rwx----
0000000080c00000 0000000080c00000 0000000000200000 rwx----
0000000080e00000 0000000080e00000 0000000000200000 rwx----
0000000081000000 0000000081000000 0000000000200000 rwx----
```
