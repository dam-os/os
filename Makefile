damos: clean
	riscv64-elf-gcc -Wall -Wextra -c -mcmodel=medany src/kernel.c -o build/kernel.o -ffreestanding
	riscv64-elf-as -c src/entry.s -o build/entry.o
	riscv64-elf-gcc -T src/linker.ld -nostdlib build/kernel.o build/entry.o -o build/kernel.elf -lgcc

run: damos 
	qemu-system-riscv64 -machine virt -bios none -kernel build/kernel.elf -serial mon:stdio

clean:
	rm -f build/*
