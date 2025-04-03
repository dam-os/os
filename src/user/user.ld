ENTRY(start);
 
. = 0x1000000;
 
SECTIONS {
	/* Include entry point at start of binary */
	.text : ALIGN(4K) {
		KEEP(*(.text.start));
		*(.init);
		*(.text);
	}
	.bss : ALIGN(4K) {
		*(.bss);
		. += 4096;
		PROVIDE(user_stack_top = .);
		. += 164 * 4096;
	}
   	.rodata : ALIGN(4K) {
		*(.rodata);
	}
	.data : ALIGN(4K) {
		*(.data);
	}
    . = ALIGN(4096);
}
