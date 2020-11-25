#ifndef K_ISR_H
#define K_ISR_H

#include <k/types.h>
#include <k/pic.h>

#include <stdio.h>

#define X(vector_number, interrupt_name) extern void isr_##vector_number(void);
#include <k/isr.def>
#undef X

#define X(vector_number, interrupt_name) INTERRUPT_##interrupt_name = vector_number,
enum INTERRUPTS_NAME {
#include <k/isr.def>
};
#undef X

struct Registers {
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
};

extern void interrupt_handler(struct Registers registers, u32 vector_number, u32 error_code) {
    printf("Received interrupt %u\n", vector_number);

    if (vector_number == INTERRUPT_DIVIDE_BY_ZERO) {
        printf("Divide by zero issued\n");
    }

    if (vector_number > 31 && vector_number < 32 + 8) {
        printf("Master PIC interrupt");
    } else if (vector_number > 31 + 8 && vector_number < 32 + 16) {
        printf("Slave PIC interrupt");
    }

    if (vector_number == INTERRUPT_IRQ_1) {
        printf("Keyboard pressed\n");

        send_EOI(vector_number);
    }

    if (error_code != 0) {
        printf("Interrupt with error occured error code : %u", error_code);
    }
}

#endif				/* !ISR_H_ */
