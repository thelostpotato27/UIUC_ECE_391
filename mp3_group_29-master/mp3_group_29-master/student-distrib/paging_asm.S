#define ASM 1

.text

.globl setPD
.globl enablePaging

# take PD address and loads into cr3
# follows C calling convention

setPD:

  pushl %ebp
  movl  %esp, %ebp
  
  # loads PD into cr3
  movl  8(%esp), %eax
  movl  %eax, %cr3

  # kernel memory map
  movl %cr4, %eax
  orl $0x00000010, %eax
  movl %eax, %cr4

  leave
  ret

enablePaging:

  pushl %ebp
  movl  %esp, %ebp

  # Enalbes paging
  movl  %cr0, %eax
  orl   $0x80000001, %eax # ENABLE_PG const
  movl  %eax, %cr0

  leave
  ret
  