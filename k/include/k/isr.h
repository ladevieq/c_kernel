#include <k/types.h>

#include <stdio.h>

#define X(function_name) extern void function_name(void);
#include <k/isr.def>
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

extern void interrupt_handler(struct Registers registers, u32 interrupt_code, void* error) {
    printf("Received interrupt %u\n", interrupt_code);
}
