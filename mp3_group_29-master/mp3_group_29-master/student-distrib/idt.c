#include "idt.h"

/* The messages that display when an exception happens. */

// TODO -- add error codes as per manual
static char *exception_messages[20] = {
    "Division Error Exception",
    "Debug Exception",
    "NMI Interrupt",
    "Breakpoint Exception",
    "Overflow Exception",
    "Bound Range Exceeded Exception",
    "Invalid Opcode Exception",
    "Device Not Available Exception",
    "Double Fault Exception",
    "Coprocessor Segment Overrun",
    "Invalid TSS Exception",
    "Segment Not Present",
    "Stack Fault Exception",
    "General Protection Exception",
    "Page Fault Exception",
    "Reserved",
    "x86 FPU Floating Point Error",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating Point Exception"};

/*
 * DESCRIPTION: Initializes IDT.
 *
 * INPUTS: None
 *
 * OUTPUTS: None
 *
 * SIDE EFFECTS: Modifies IDT.
 */

void init_IDT(void)
{
    int i;
    for (i = 0; i < NUM_VEC; i++)
    {
        // kernel code -- see x86_desc.h for details
        idt[i].seg_selector = KERNEL_CS;

        /* 01110 in the reserved section is an exception,
         * 01100 is user-level interrupt. Exceptions should
         * always have ring 0 permissions (kernel level),
         * but syscalls should have ring 3 permissions (user level).
         */

        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        // idt[i].reserved3 = (i >= INTERRUPT_VECTOR_TABLE_START && i <= INTERRUPT_VECTOR_TABLE_END) ? 0 : 1;
        idt[i].reserved4 = 0;

        // idt[i].dpl = (i == SYSCALL_IDX) ? 3 : 0;

        if (i < 32)
        {
            idt[i].reserved3 = 1;
            idt[i].dpl = 0; // ring 0 perms
        }                   // first 32 reserved by Intel
        else
        {
            idt[i].reserved3 = 0;
            idt[i].dpl = 3;
        }
        idt[i].present = 1; // will be set by SET_IDT_ENTRY, default not present
        idt[i].size = 1;    // 32 bits
    }

    // Exceptions -- see idt_wrap.S.
    SET_IDT_ENTRY(idt[0], divide_error_exception);
    SET_IDT_ENTRY(idt[1], debug_exception);
    SET_IDT_ENTRY(idt[2], nmi_interrupt);
    SET_IDT_ENTRY(idt[3], breakpoint_exception);
    SET_IDT_ENTRY(idt[4], overflow_exception);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_exception);
    SET_IDT_ENTRY(idt[6], invalid_opcode_exception);
    SET_IDT_ENTRY(idt[7], device_not_available_exception);
    SET_IDT_ENTRY(idt[8], double_fault_exception);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_tss_exception);
    SET_IDT_ENTRY(idt[11], segment_not_present);
    SET_IDT_ENTRY(idt[12], stack_fault_exception);
    SET_IDT_ENTRY(idt[13], general_protection_exception);
    SET_IDT_ENTRY(idt[14], page_fault_exception);
    SET_IDT_ENTRY(idt[16], x86_fpu_floating_point_error);
    SET_IDT_ENTRY(idt[17], alignment_check_exception);
    SET_IDT_ENTRY(idt[18], machine_check_exception);
    SET_IDT_ENTRY(idt[19], simd_floating_point_exception);

    // Devices
    SET_IDT_ENTRY(idt[PIT_IDX], pit_interrupt);
    SET_IDT_ENTRY(idt[KEYBOARD_IDX], keyboard_interrupt);
    SET_IDT_ENTRY(idt[RTC_IDX], rtc_interrupt);

    // Syscall
    SET_IDT_ENTRY(idt[SYSCALL_IDX], syscall_wrap);
}

/*
 * DESCRIPTION: Handles exceptions.
 *
 * INPUTS: None
 *
 * OUTPUTS: None
 *
 * SIDE EFFECTS: Prints error message and halts machine.
 */

// TODO -- confirm function signatures and params

void exception_handler(uint32_t index)
{
    if (index > 19)
        return;

    printf("%s\n", exception_messages[index]);
    printf("Squashing user level program and returning control to shell...\n");
    // printf("RESULT = PASS\n");
    halt(255);
}
