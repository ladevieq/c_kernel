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
    set_interrupt_gate_IDT(0, (u32) &isr_0); // Divide by zero
    set_interrupt_gate_IDT(3, (u32) &isr_3);
    // set_interrupt_gate_IDT(8, (u32) &isr_8);
    set_interrupt_gate_IDT(10, (u32) &isr_10);
    set_interrupt_gate_IDT(17, (u32) &isr_17);
    set_interrupt_gate_IDT(INTERRUPT_TIMER_IRQ, (u32) &isr_32);
    set_interrupt_gate_IDT(INTERRUPT_KEYBOARD_IRQ, (u32) &isr_33);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_2, (u32) &isr_34);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_3, (u32) &isr_35);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_4, (u32) &isr_36);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_5, (u32) &isr_37);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_6, (u32) &isr_38);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_7, (u32) &isr_39);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_8, (u32) &isr_40);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_9, (u32) &isr_41);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_10, (u32) &isr_42);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_11, (u32) &isr_43);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_12, (u32) &isr_44);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_13, (u32) &isr_45);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_14, (u32) &isr_46);
    set_interrupt_gate_IDT(INTERRUPT_IRQ_15, (u32) &isr_47);

    set_IDT();
}
