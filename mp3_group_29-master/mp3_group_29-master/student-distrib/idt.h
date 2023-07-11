/*
 * idt.h - Initializes interrupt descriptor table (IDT),
 * handles interrupts and exceptions.
 * vim:ts=4 noexpandtab
 */

#ifndef IDT_H
#define IDT_H


#ifndef ASM

#include "x86_desc.h"
#include "syscalls.h"
#include "lib.h"
#include "types.h"
#include "i8253.h"

/*Refer to IA-32 manual for position in IDT vector table. */
#define PIT_IDX 0x20
#define KEYBOARD_IDX 0x21
#define RTC_IDX 0x28
#define SYSCALL_IDX 0x80

/*Start and end points of interrupt vector table section in idt.*/
#define INTERRUPT_VECTOR_TABLE_START 0x20 
#define INTERRUPT_VECTOR_TABLE_END 0x2F



    /* Initializes IDT. */
    void init_IDT(void);

    /* Handles exceptions. */
    void exception_handler(uint32_t index);


    /* Signatures for interrupts. */
    void divide_error_exception(void); /*idt[0]*/
    void debug_exception(void); /*idt[1]*/
    void nmi_interrupt(void); /*idt[2]*/
    void breakpoint_exception(void); /*idt[3]*/
    void overflow_exception(void); /*idt[4]*/
    void bound_range_exceeded_exception(void); /*idt[5]*/
    void invalid_opcode_exception(void); /*idt[6]*/
    void device_not_available_exception(void); /*idt[7]*/
    void double_fault_exception(void); /*idt[8]*/
    void coprocessor_segment_overrun(void); /*idt[9]*/
    void invalid_tss_exception(void); /*idt[10]*/
    void segment_not_present(void); /*idt[11]*/
    void stack_fault_exception(void); /*idt[12]*/
    void general_protection_exception(void); /*idt[13]*/
    void page_fault_exception(void); /*idt[14]*/
    /*idt[15] reserved*/
    void x86_fpu_floating_point_error(void); /*idt[16]*/
    void alignment_check_exception(void); /*idt[17]*/
    void machine_check_exception(void); /*idt[18]*/
    void simd_floating_point_exception(void); /*idt[19]*/
    
    /*Devices.*/
    void pit_interrupt(void); /*idt[PIT_IDX]*/
    void keyboard_interrupt(void); /*idt[KEYBOARD_IDX]*/
    void rtc_interrupt(void);  /*idt[RTC_IDX]*/

    /*System call wrapper signature should be in syscalls.h.*/

#endif // ASM

#endif // IDT_H
