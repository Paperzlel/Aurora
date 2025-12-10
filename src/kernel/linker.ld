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
  . += virt;
  
  /* All here is within virtual addresses. */
  .text  : AT(ADDR(.text) - virt) ALIGN(4K)
  {
    __text_start = .;        *(.text)      
  }
  __text_end = .;
  
  .data  : AT(ADDR(.data) - virt) ALIGN(4K)
  {
    __data_start = .;        *(.data)      
  }
  __data_end = .;
  
  .rodata  : AT(ADDR(.rodata) - virt) ALIGN(4K)
  {
    __rodata_start = .;       *(.rodata)
  }
  __rodata_end = .;
  
  .bss  : AT(ADDR(.bss) - virt) ALIGN(4K)
  {
    __bss_start = .;         *(.bss)
  }
  __bss_end = .;

  . = ALIGN(4K);
  __end = .;
}
