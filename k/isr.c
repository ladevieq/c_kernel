#include <k/isr.h>
#include <k/keyboard.h>
#include <k/pic.h>
#include <k/timer.h>

#include <stdio.h>

void interrupt_handler(struct Registers registers, u32 vector_number, u32 error_code) {
    // printf("Received interrupt %u\n", vector_number);

    if (vector_number == INTERRUPT_DIVIDE_BY_ZERO) {
        printf("Divide by zero issued\n");
    }

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

    if (error_code != 0) {
        printf("Interrupt with error occured error code : %u", error_code);
    }
}
