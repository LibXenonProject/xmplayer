ENTRY(_start)
SECTIONS
{
	. = 0x80000000;
	.text : {  *(.except*); *(.pagetable); pagetable_end = .; *(.text*); *(.init*); }
	.elfldr : { elfldr_start = .;*(.elfldr); elfldr_end = .;}
	
	.data : { *(.data*) }
	.sdata : { *(.sdata*) }
	.rodata : { *(.rodata*)}

	.ctors : {
		__CTOR_LIST__ = .; 
		*(.ctors); 
		__CTOR_END__ = .;
		. = ALIGN(32);
	}
	.dtors : {
		__DTOR_LIST__ = .; 
		*(.dtors); 
		__DTOR_END__ = .;
		. = ALIGN(32);
	}

	.eh_frame_hdr : { *(.eh_frame_hdr) }
	.eh_frame ALIGN(0x4): { PROVIDE (__eh_frame_start = .); KEEP (*(.eh_frame)) LONG (0); }
	.gcc_except_table :  { *(.gcc_except_table .gcc_except_table.*) }

	bss_start = .;
	.bss : { *(.bss*) }
	.sbss : { *(.sbss*) }
	bss_end = .;

	. = ALIGN(32);
	heap_begin = .;
	. = 0x9E000000;
	__libc_stack_end = . ;
	heap_end = .;
	. = 0xa0000000;
}
