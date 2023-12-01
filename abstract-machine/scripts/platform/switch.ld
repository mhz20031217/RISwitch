ENTRY(_start)
PHDRS { text PT_LOAD; data PT_LOAD; }

MEMORY {
  ROM : ORIGIN = 0x80000000, LENGTH = 128K
  RAM : ORIGIN = 0x80100000, LENGTH = 512K
}

SECTIONS {
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : {
    *(entry)
    *(.text*)
  } > ROM : text
  etext = .;
  _etext = .;
  . = 0x80100000;
  .rodata : {
    *(.srodata*)
    *(.rodata*)
  } > RAM : data
  .data : {
    *(.data)
  } > RAM : data
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } > RAM
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
}
