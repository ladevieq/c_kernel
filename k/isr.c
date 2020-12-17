#include <k/isr.h>
#include <k/keyboard.h>
#include <k/pic.h>
#include <k/timer.h>
#include <k/syscalls.h>

#include <stdio.h>

void print_int_info(u32 int_number, u32 error_code) {
#define X(vector_number, interrupt_name) if (int_number == vector_number) { \
    printf("%s", #interrupt_name);                                          \
}
#include <k/isr.def>
#undef X

    if (error_code != 0) {
        printf(" error code : %u", error_code);
    }

    printf("\n");
}

void interrupt_handler(struct IntHandlerArgs* args) {
    u32 vec_number = args->vector_number;
    u32 error_code = args->error_code;
    print_int_info(vec_number, error_code);

    // PIC interrupts
    if (vec_number >= MASTER_PIC_VECTOR_OFFSET
        && vec_number < SLAVE_PIC_VECTOR_OFFSET + 7) {
        if (vec_number == INTERRUPT_TIMER_IRQ) {
            tick();
        }

        if (vec_number == INTERRUPT_KEYBOARD_IRQ) {
            pullkey();

            s32 key = getkey();
            if (key > 0) {
                printf("%u ms elapsed since startup\n", gettick());
            }
        }

        send_EOI(vec_number);
    }

    if (vec_number == INTERRUPT_SYSCALL) {
        args->registers.eax = dispatch_syscall(
            args->registers.eax,
            args->registers.ebx,
            args->registers.ecx,
            args->registers.edx
        );
    }
}
