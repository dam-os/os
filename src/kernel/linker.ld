ENTRY(start);
 
. = 0x84000000;
 
SECTIONS {
	__kernel_base = .;
	/* Include entry point at start of binary */
	.text : ALIGN(4K) {
		*(.init);
		*(.text);
	}
	.bss : ALIGN(4K) {
		PROVIDE(bss_start = .);
		*(.bss);
		. += 4096;
		PROVIDE(stack_top = .);
		. += 64 * 4096;
		PROVIDE(bss_end = .);
	}
   	.rodata : ALIGN(4K) {
		*(.rodata);
	}
	.data : ALIGN(4K) {
		*(.data);
	}

   .dtb : ALIGN(4K) {
        PROVIDE(__dtb_start = .);
        KEEP(*(.dtb_blob))
        PROVIDE(__dtb_end = .);
    }

    . = ALIGN(4096);
    __free_ram = .;
    . += 64 * 1024 * 1024;
    __free_ram_end = .;
   }
