#define ASM 1
#include "x86_desc.h"

#define STACK   0x8400000
#define INTERRUPT_FLAG 0x200

.globl syscall_wrap
.globl context_switch
.globl move_args

syscall_wrap:

	pushl $0
	pushfl
	# saves registers except for eax
	pushl %ebx
	pushl %ecx
	pushl %edx
    pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
	
	# syscalls begin at index 1
	cmpl $0, %eax
	jbe invalid_call
	cmpl $10, %eax
	ja invalid_call

	# pushes args onto stack -- not all may be used

    pushl %edx
    pushl %ecx
    pushl %ebx

	dec %eax # get offset

	call *syscall_table(, %eax, 4) # return value in eax

	addl $12, %esp # pops args

	jmp done

invalid_call:
	# return -1 for invalid call
	movl $-1, %eax

done:

	popl %edi
	popl %esi
	popl %ebp
    popl %esp
	popl %edx
	popl %ecx
	popl %ebx

	popfl

	addl $4, %esp

	iret

context_switch: # setup for IRET

    # Load entry point in EBX
    movl 4(%esp),%ebx

    # Push SS on stack
    xorl %eax, %eax
    movw $USER_DS, %ax
    pushl %eax

    # Push ESP value for user stack (page fault)
    movl  $STACK, %eax
    pushl %eax

    # Push eflags on stack
    pushfl

    # Set IF flag in eflags
    popl  %eax
    orl   $INTERRUPT_FLAG,%eax
    pushl %eax

    # Push CS on stack
    pushl $USER_CS

    # Push EIP for user program
    pushl %ebx

    iret

.align 4
move_args:
    pushl %ebp
    movl %esp, %ebp

    xorl %ebx, %ebx
    movl 16(%ebp), %ebx     # get third arg (status)
    movl %ebx, %eax         

    movl 12(%ebp), %ebx     # get second arg (esp)
    movl %ebx, %esp   

    movl 8(%ebp), %ebx       # get first arg (ep)
    movl %ebx, %ebp

    leave
    ret

.align 4

syscall_table:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn 
    