#include <k/isr.h>
#include <k/keyboard.h>
#include <k/pic.h>
#include <k/timer.h>

#include <stdio.h>

void print_int_info(u32 int_number, u32 error_code) {
#define X(vector_number, interrupt_name) if (int_number == vector_number) { \
    printf("%s", #interrupt_name);                                                   \
}
#include <k/isr.def>
#undef X

    if (error_code != 0) {
        printf(" error code : %u", error_code);
    }

    printf("\n");
}

void interrupt_handler(struct Registers registers, u32 vector_number, u32 error_code) {
    print_int_info(vector_number, error_code);

    if (vector_number == INTERRUPT_DIVIDE_BY_ZERO) {
        printf("Divide by zero issued\n");
    }

    // PIC interrupts
    if (vector_number >= MASTER_PIC_VECTOR_OFFSET
        && vector_number < SLAVE_PIC_VECTOR_OFFSET + 7) {
        if (vector_number == INTERRUPT_TIMER_IRQ) {
            tick();
        }

        if (vector_number == INTERRUPT_KEYBOARD_IRQ) {
            pullkey();

            s32 key = getkey();
            if (key > 0) {
                // printf("%d key pressed\n", key);
                printf("%u ms elapsed since startup\n", gettick());
            }
        }

        send_EOI(vector_number);
    }
}
