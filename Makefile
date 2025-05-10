SRCDIR = src/kernel
BUILDDIR = build
LIBDIR = $(SRCDIR)/lib
MEMDIR = $(SRCDIR)/memory
DRIVERDIR = $(SRCDIR)/drivers
USERDIR = src/user
TESTDIR = test
TESTBUILDDIR = $(BUILDDIR)/test

# Source files
KERNEL_SRC = $(SRCDIR)/kernel.c
C_SOURCES = $(filter-out $(KERNEL_SRC), $(wildcard $(SRCDIR)/*.c)) $(wildcard $(LIBDIR)/*.c) $(wildcard $(MEMDIR)/*.c) $(wildcard $(DRIVERDIR)/*.c)
ASM_SOURCES = $(wildcard $(SRCDIR)/*.s)

# Test-specific files
TEST_KERNEL_SRC = $(TESTDIR)/test_kernel.c
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_SOURCES := $(filter-out $(TEST_KERNEL_SRC), $(TEST_SOURCES))

# Object files
C_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRCDIR)/%.s, $(BUILDDIR)/%.o, $(ASM_SOURCES))
KERNEL_OBJECT = $(BUILDDIR)/kernel.o
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c, $(TESTBUILDDIR)/%.o, $(TEST_SOURCES))
TEST_KERNEL_OBJECT = $(TESTBUILDDIR)/test_kernel.o

# Compiler settings
CC = riscv64-elf-gcc
AS = riscv64-elf-as
OBJCOPY = riscv64-elf-objcopy
CFLAGS = -Wall -Wextra -c -mcmodel=medany -ffreestanding -ggdb
LDFLAGS = -T $(SRCDIR)/linker.ld -nostdlib -lgcc
USERLDFLAGS = -T $(USERDIR)/user.ld -O2 -g3 -Wall -Wextra -fno-stack-protector -ffreestanding -nostdlib

# Qemu settings
QEMU = qemu-system-riscv64
define QFLAGS
		-machine virt \
		-bios none \
		-drive id=drive0,file=file.txt,format=raw,if=none \
        -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
		-device virtio-vga \
		-kernel $(BUILDDIR)/kernel.elf \
		-cpu rv64,pmp=false \
		-serial mon:stdio
endef
define QFLAGS-SPL
		-machine virt \
		-cpu rv64,pmp=false \
		-serial mon:stdio \
		-bios fw_jump.bin \
		-kernel u-boot.bin \
		-device loader,file=build/kernel.bin,addr=0x84000000 \
		-device loader,file=output.dtb,addr=0x88000000 
endef

# Main kernel build (uses kernel.c)
damos: clean build_dirs $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS)
	$(CC) $(USERLDFLAGS) -o $(BUILDDIR)/shell.elf $(USERDIR)/shell.c $(USERDIR)/user.c
	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary $(BUILDDIR)/shell.elf $(BUILDDIR)/shell.bin
	$(OBJCOPY) -Ibinary -Oelf64-littleriscv $(BUILDDIR)/shell.bin $(BUILDDIR)/shell.bin.o

	$(CC) $(LDFLAGS) $(BUILDDIR)/shell.bin.o $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) -o $(BUILDDIR)/kernel.elf
	
	riscv64-elf-objcopy -O binary $(BUILDDIR)/kernel.elf $(BUILDDIR)/kernel.bin

# Test kernel build (uses test_kernel.c)
test_kernel: clean build_dirs $(TEST_KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) $(TEST_KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) $(TEST_OBJECTS) -o $(TESTBUILDDIR)/test_kernel.elf

# Ensure build directories exist
build_dirs:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/lib $(BUILDDIR)/memory $(BUILDDIR)/drivers $(TESTBUILDDIR)

# Compilation rules
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(LIBDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(MEMDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(DRVDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s | build_dirs
	$(AS) -c $< -o $@

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

# Run main kernel
run: damos
	$(QEMU) $(QFLAGS)

tmux: damos
	@tmux split-window -h
	@tmux send-keys "gdb -ex 'target remote localhost:1234' -ex 'symbol-file ./build/kernel.elf' -ex 'break *kmain' -ex 'c'" C-m
	$(QEMU) $(QFLAGS) -s -S -nographic

debug: damos
	$(QEMU) $(QFLAGS) -s -S -nographic

# Run test kernel in QEMU
run_test: test_kernel
	qemu-system-riscv64 -machine virt -bios none -kernel $(TESTBUILDDIR)/test_kernel.elf -serial mon:stdio

# Cleanup
clean:
	rm -rf $(BUILDDIR)/*

sdcard:
	$(QEMU) $(QFLAGS-SPL)

sdcard2:
	$(QEMU) $(QFLAGS-SPL) -s -S
