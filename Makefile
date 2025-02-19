SRCDIR = src
BUILDDIR = build
LIBDIR = $(SRCDIR)/lib

C_SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(LIBDIR)/*.c)
ASM_SOURCES = $(wildcard $(SRCDIR)/*.s)

C_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRCDIR)/%.s, $(BUILDDIR)/%.o, $(ASM_SOURCES))

CC = riscv64-elf-gcc
AS = riscv64-elf-as
CFLAGS = -Wall -Wextra -c -mcmodel=medany -ffreestanding
LDFLAGS = -T $(SRCDIR)/linker.ld -nostdlib -lgcc

damos: clean build_dirs $(C_OBJECTS) $(ASM_OBJECTS)
	$(CC) $(LDFLAGS) $(C_OBJECTS) $(ASM_OBJECTS) -o $(BUILDDIR)/kernel.elf

build_dirs:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/lib

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(LIBDIR)/%.c | build_dirs
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s | build_dirs
	$(AS) -c $< -o $@

run: damos
	qemu-system-riscv64 -machine virt -bios none -kernel $(BUILDDIR)/kernel.elf -serial mon:stdio

clean:
	rm -rf $(BUILDDIR)/*

