#define ASM 1
#
# x86 interrupts - see IA-32 Architecture Software 
# Developer's Manual Volume 3 for reference. Interrupts
# are listed in ascending numerical order.
#

# System Interrupts - idt[0-19]
.globl divide_error_exception
.globl debug_exception
.globl nmi_interrupt
.globl breakpoint_exception
.globl overflow_exception
.globl bound_range_exceeded_exception
.globl invalid_opcode_exception
.globl device_not_available_exception
.globl double_fault_exception
.globl coprocessor_segment_overrun
.globl invalid_tss_exception
.globl segment_not_present
.globl stack_fault_exception
.globl general_protection_exception
.globl page_fault_exception
# idt[15] reserved 
.globl x86_fpu_floating_point_error
.globl alignment_check_exception
.globl machine_check_exception
.globl simd_floating_point_exception

# Devices
.globl pit_interrupt
.globl keyboard_interrupt
.globl rtc_interrupt

# Exceptions and Interrupts

# Each interrupt saves all registers, then
# pushes the array index of the error message
# to the stack. The exception handler (in idt.c)
# will print the appropriate error message and
# halt the machine. Registers will be restored
# and the system returns from the interrupt.

divide_error_exception:
    pushal 
    pushl $0
    jmp exception_wrap
debug_exception:
    pushal 
    pushl $1
    jmp exception_wrap
nmi_interrupt:
    pushal 
    pushl $2
    jmp exception_wrap
breakpoint_exception:
    pushal 
    pushl $3
    jmp exception_wrap
overflow_exception:
    pushal 
    pushl $4
    jmp exception_wrap
bound_range_exceeded_exception:
    pushal 
    pushl $5
    jmp exception_wrap
invalid_opcode_exception:
    pushal 
    pushl $6
    jmp exception_wrap
device_not_available_exception:
    pushal 
    pushl $7
    jmp exception_wrap
double_fault_exception:
    pushal 
    pushl $8
    jmp exception_wrap
coprocessor_segment_overrun:
    pushal 
    pushl $9
    jmp exception_wrap
invalid_tss_exception:
    pushal 
    pushl $10
    jmp exception_wrap
segment_not_present:
    pushal 
    pushl $11
    jmp exception_wrap
stack_fault_exception:
    pushal 
    pushl $12
    jmp exception_wrap
general_protection_exception:
    pushal 
    pushl $13
    jmp exception_wrap
page_fault_exception:
    pushal 
    pushl $14
    jmp exception_wrap
# idt[15] reserved
x86_fpu_floating_point_error:
    pushal 
    pushl $16
    jmp exception_wrap
alignment_check_exception:
    pushal 
    pushl $17
    jmp exception_wrap
machine_check_exception:
    pushal 
    pushl $18
    jmp exception_wrap
simd_floating_point_exception:
    pushal 
    pushl $19
    jmp exception_wrap

# Devices -- these will also preserve
# the flags.

# TODO: ask for names of the handlers

pit_interrupt:
    cli
    pushal
    pushfl 
    call pit_handler
    popfl
    popal
    sti
    iret

keyboard_interrupt:
    cli
    pushal
    pushfl 
    call keyboard_handler
    popfl
    popal
    sti
    iret

rtc_interrupt:
    cli
    pushal
    pushfl 
    call rtc_handler
    popfl
    popal
    sti
    iret

exception_wrap:
    call exception_handler

    # exceptions cause a HALT

    addl $4, %esp # pops arg
    popal # restores all registers
    iret
