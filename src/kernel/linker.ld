ENTRY(start)
OUTPUT_FORMAT("elf32-i386")
phys = 0x00100000;
virt = 0xc0000000;

SECTIONS
{
  /* Load kernel at physical address 0x0010 0000. Enable paging immediately, jump to higher half and continue. */
  . = phys;

  __kernel_lh_start = .;

  .kernel_lh.data :
  {
    __kernel_lh_data_start = .; *(.kernel_lh.data)
  }

  .kernel_lh.text : 
  { 
    __kernel_lh_text_start = .; *(.kernel_lh.text) 
  }


  __kernel_lh_end = .;
  phys = __kernel_lh_end;
  . = ASSERT(phys == __kernel_lh_end, "Invalid size of kernel lower half.");
  . = virt;
  
  /* All here is within virtual addresses. */
  .text  : AT(ADDR(.text) - virt + phys)
  {
    __text_start = .;        *(.text)      
  }
  
  .data  : AT(ADDR(.data) - virt + phys)
  {
    __data_start = .;        *(.data)      
  }
  
  .rodata  : AT(ADDR(.rodata) - virt + phys)
  {
    __entry_start = .;       *(.rodata)
  }
  
  .bss  : AT(ADDR(.bss) - virt + phys)
  {
    __bss_start = .;         *(.bss)
  }

  __end = .;
}
