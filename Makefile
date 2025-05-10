BOARD ?= virt
BUILD_UBOOT ?= true

SRCDIR = src/kernel
BUILDDIR = build
LIBDIR = $(SRCDIR)/lib
MEMDIR = $(SRCDIR)/memory
DRIVERDIR = $(SRCDIR)/drivers
USERDIR = src/user
DTS = $(SRCDIR)/device_tree/virt.dts
DTB_OBJ = $(BUILDDIR)/dtb.o

# Set DTB based on the BOARD value
ifeq ($(BOARD),virt)
DTS = $(SRCDIR)/device_tree/virt.dts
else ifeq ($(BOARD),board)
DTS = $(SRCDIR)/device_tree/deepcomp.dts
else
$(error Unsupported BOARD value: $(BOARD))
endif

DTB = $(BUILDDIR)/$(notdir $(DTS:.dts=.dtb))
DTB_OBJ = $(BUILDDIR)/dtb.o

# Compile rules
all: $(DTB_OBJ)

# Compile DTS to DTB
$(DTB): $(DTS)
	dtc -q -I dts -O dtb -o $@ $<

# Convert DTB to object file
$(DTB_OBJ): $(DTB)
	ld -r -b binary -o $@ $<
# Source files
KERNEL_SRC = $(SRCDIR)/kernel.c
C_SOURCES = $(filter-out $(KERNEL_SRC), $(wildcard $(SRCDIR)/*.c)) $(wildcard $(LIBDIR)/*.c) $(wildcard $(MEMDIR)/*.c) $(wildcard $(DRIVERDIR)/*.c)
ASM_SOURCES = $(wildcard $(SRCDIR)/*.s)

# Object files
C_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRCDIR)/%.s, $(BUILDDIR)/%.o, $(ASM_SOURCES))
KERNEL_OBJECT = $(BUILDDIR)/kernel.o

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
		-device virtio-vga \
		-kernel $(BUILDDIR)/kernel.elf \
		-cpu rv64,pmp=false \
		-serial mon:stdio
endef
define QFLAGS-MACHINE
		-machine virt \
		-cpu rv64,pmp=false \
		-serial mon:stdio \
		-device virtio-vga \
		-bios opensbi/build/platform/generic/firmware/fw_jump.bin \
		-kernel u-boot/u-boot.bin \
		-device loader,file=build/kernel.bin,addr=0x84000000
endef

# Main kernel build (uses kernel.c)
damos: clean build_dirs $(DTB_OBJ)  $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS)
	$(CC) $(USERLDFLAGS) -o $(BUILDDIR)/shell.elf $(USERDIR)/shell.c $(USERDIR)/user.c
	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary $(BUILDDIR)/shell.elf $(BUILDDIR)/shell.bin
	$(OBJCOPY) -Ibinary -Oelf64-littleriscv $(BUILDDIR)/shell.bin $(BUILDDIR)/shell.bin.o

	$(CC) $(LDFLAGS) $(BUILDDIR)/shell.bin.o $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) -o $(BUILDDIR)/kernel.elf
	$(OBJCOPY) -I binary -O elf64-littleriscv -B riscv --rename-section .data=.dtb_blob $(DTB) $(DTB_OBJ)
	$(CC) $(LDFLAGS) $(DTB_OBJ) $(BUILDDIR)/shell.bin.o $(KERNEL_OBJECT) $(C_OBJECTS) $(ASM_OBJECTS) -o $(BUILDDIR)/kernel.elf
	riscv64-elf-objcopy -O binary $(BUILDDIR)/kernel.elf $(BUILDDIR)/kernel.bin

# Ensure build directories exist
build_dirs:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/lib $(BUILDDIR)/memory $(BUILDDIR)/drivers

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

# Run main kernel
run: damos
	$(QEMU) $(QFLAGS)

tmux: damos
	@tmux split-window -h
	@tmux send-keys "gdb -ex 'target remote localhost:1234' -ex 'symbol-file ./build/kernel.elf' -ex 'break *kmain' -ex 'c'" C-m
	$(QEMU) $(QFLAGS) -s -S -nographic

debug: damos
	$(QEMU) $(QFLAGS) -s -S -nographic

# Cleanup
clean:
	rm -rf $(BUILDDIR)/*

open-sbi:
	$(MAKE) -C opensbi PLATFORM=generic FW_TEXT_START=0x40000000 FW_OPTIONS=0

build-uboot: open-sbi
ifeq ($(BOARD),virt)
	$(MAKE) -C u-boot qemu-riscv64_smode_defconfig
else
	$(MAKE) -C u-boot starfive_visionfive2_defconfig
endif
	MAKEFLAGS= $(MAKE) -C u-boot OPENSBI=../opensbi/build/platform/generic/firmware/fw_dynamic.bin

u-boot: damos open-sbi
	$(QEMU) $(QFLAGS-MACHINE)

u-boot-debug: damos open-sbi 
	$(QEMU) $(QFLAGS-MACHINE) -s -S
