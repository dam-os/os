SRCDIR = src
BUILDDIR = build
LIBDIR = $(SRCDIR)/lib
TESTDIR = test
TESTBUILDDIR = $(BUILDDIR)/test

# Source files
KERNEL_SRC = $(SRCDIR)/kernel.c
C_SOURCES = $(filter-out $(KERNEL_SRC), $(wildcard $(SRCDIR)/*.c)) $(wildcard $(LIBDIR)/*.c)
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
CFLAGS = -Wall -Wextra -c -mcmodel=medany -ffreestanding -ggdb
LDFLAGS = -T $(SRCDIR)/linker.ld -nostdlib -lgcc

# Main kernel build (uses kernel.c)
damos: clean build_dirs $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS)
	$(CC) $(LDFLAGS) $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) -o $(BUILDDIR)/kernel.elf

# Test kernel build (uses test_kernel.c)
test_kernel: clean build_dirs $(TEST_KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) $(TEST_KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) $(TEST_OBJECTS) -o $(TESTBUILDDIR)/test_kernel.elf

# Ensure build directories exist
build_dirs:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/lib $(TESTBUILDDIR)

# Compilation rules
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(LIBDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s | build_dirs
	$(AS) -c $< -o $@

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

# Run main kernel
run: damos
	qemu-system-riscv64 \
		-machine virt \
		-bios none \
		-drive id=drive0,file=file.txt,format=raw,if=none \
<<<<<<< HEAD
    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
		-kernel $(BUILDDIR)/kernel.elf \
		-serial mon:stdio
=======
		-device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
		-kernel $(BUILDDIR)/kernel.elf \
		-serial mon:stdio \

debug: damos
	qemu-system-riscv64 \
		-machine virt \
		-bios none \
		-drive id=drive0,file=file.txt,format=raw,if=none \
		-device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
		-kernel $(BUILDDIR)/kernel.elf \
		-serial mon:stdio \
		-s -S
>>>>>>> main

# Run test kernel in QEMU
run_test: test_kernel
	qemu-system-riscv64 -machine virt -bios none -kernel $(TESTBUILDDIR)/test_kernel.elf -serial mon:stdio

# Cleanup
clean:
	rm -rf $(BUILDDIR)/*
