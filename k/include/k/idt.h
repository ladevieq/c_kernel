#ifndef K_IDT_H
#define K_IDT_H

#include <k/types.h>

#define INTERRUPTS_COUNT 255
#define INTERRUPT_GATE 0b110
#define TRAP_GATE 0b111

struct Gate {
    union {
        struct {
            u16 offset;
            u16 segment_selector;
            u8 reserved: 5;
            u8 unused: 3;
            u8 gate_type: 3;
            u8 gate_size: 1;
            u8 zero: 1;
            u8 descriptor_privilege_level: 2;
            u8 segment_present: 1;
            u16 extended_offset;
        }__attribute__((packed));
        u64 data;
    };
};

struct InterruptDescriptorTable {
    struct Gate descriptors[INTERRUPTS_COUNT];
};

struct IDTR {
    u16 limit;
    struct InterruptDescriptorTable* base;
}__attribute__((packed));


void init_IDT();

#endif				/* !IDT_H_ */
