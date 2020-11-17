#include <k/types.h>

#include <stdio.h>

#define X(interrupt_code) extern void isr_##interrupt_code(void);
#include <k/isr.def>
#undef X

// #define X(interrupt_code, interrupt_name) INTERRUPT_##interrupt_name = interrupt_code,
// enum INTERRUPTS_NAME {
// #include <k/isr.def>
// };
// #undef X

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

extern void interrupt_handler(struct Registers registers, u32 interrupt_code, u32 error_code) {
    printf("Received interrupt %u\n", interrupt_code);

    if (error_code != 0) {
        printf("Interrupt with error occured error code : %u", error_code);
    }
}
