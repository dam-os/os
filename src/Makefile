damos:
	riscv64-elf-gcc -Wall -Wextra -c -mcmodel=medany kernel.c -o kernel.o -ffreestanding
	riscv64-elf-as -c entry.s -o entry.o
	riscv64-elf-gcc -T linker.ld -nostdlib kernel.o entry.o -o kernel.elf -lgcc

run: 
	qemu-system-riscv64 -machine virt -bios none -kernel kernel.elf -serial mon:stdio

clean:
	rm -f *.o kernel.elf
