#ifndef K_ISR_H
#define K_ISR_H

#include <k/types.h>

#define X(vector_number, interrupt_name) extern void isr_##vector_number(void);
#include <k/isr.def>
#undef X

#define X(vector_number, interrupt_name) INTERRUPT_##interrupt_name = vector_number,
enum INTERRUPTS_NAME {
#include <k/isr.def>
};
#undef X

struct IntHandlerArgs {
    struct Registers {
        u32 edi;
        u32 esi;
        u32 ebp;
        u32 esp;
        u32 ebx;
        u32 edx;
        u32 ecx;
        u32 eax;
    } registers;
    u32                 vector_number;
    u32                 error_code;
};


void interrupt_handler(struct IntHandlerArgs* args);

#endif				/* !ISR_H_ */
