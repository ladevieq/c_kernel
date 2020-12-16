#include <k/idt.h>
#include <k/gdt.h>
#include <k/isr.h>

static struct InterruptDescriptorTable IDT;

void set_IDT() {
    struct IDTR idtr;
    idtr.base = &IDT;
    idtr.limit = sizeof(struct InterruptDescriptorTable) - 1;

    asm volatile("lidt %0\n" : : "m" (idtr) : "memory");
}

void set_interrupt_gate_IDT(u8 interrupt_index, u32 offset) {
    struct Gate interrupt_gate = { 0 };
    interrupt_gate.offset = (u16)offset;
    interrupt_gate.segment_selector = KERNEL_CODE_SELECTOR;
    interrupt_gate.gate_type = INTERRUPT_GATE;
    interrupt_gate.gate_size = 1;
    interrupt_gate.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    interrupt_gate.segment_present = 1;
    interrupt_gate.extended_offset = (u16)((u32)offset >> 16);

    IDT.descriptors[interrupt_index] = interrupt_gate;
}

void set_trap_gate_IDT(u8 gate_index, u32 offset) {
    struct Gate trap_gate = { 0 };
    trap_gate.offset = (u16)offset;
    trap_gate.segment_selector = KERNEL_CODE_SELECTOR;
    trap_gate.gate_type = TRAP_GATE;
    trap_gate.gate_size = 1;
    trap_gate.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    trap_gate.segment_present = 1;
    trap_gate.extended_offset = (u16)(offset >> 16);

    IDT.descriptors[gate_index] = trap_gate;
}

void init_IDT() {
#define X(vector_number, interrupt_name) set_interrupt_gate_IDT(vector_number, (u32) &isr_##vector_number); // ##interrupt_name
#include <k/isr.def>
#undef X

    set_IDT();
}
