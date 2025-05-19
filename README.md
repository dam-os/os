# DAM-OS an educational RISC-V operating system

## Running the OS

The OS uses U-Boot and with OpenSBI. You will also need a gcc crosscompiler for RISC-V.
You can use the nix flake with:

```
nix develop .#
```

With the tools installed you can build the os with:

```
make build-uboot BOARD=virt
```

Then run the os with

```
make run BOARD=virt
```

## Debugging

Add the -s and -S to enable debugging and stop at first instruction

`qemu-system-riscv64 -machine virt -bios none -kernel build/kernel.elf -serial mon:stdio -s -S`

run GDB and with the kernel

`gdb build/kernel.elf`

Attach to the remote qemu

`target remote localhost:1234`

fx break at kmain
`break *kmain`
