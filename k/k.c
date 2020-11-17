/*
 * Copyright (c) LSE
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY LSE AS IS AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL LSE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <k/kstd.h>
#include <k/types.h>
#include <k/io.h>
#include <k/isr.h>

#include <stdio.h>

#include "multiboot.h"

// GDT related stuff --------------------------------------
enum PRIVILEGE_LEVEL {
    PRIVILEGE_KERNEL_LEVEL = 0,
    PRIVILEGE_DRIVER_LEVEL_1,
    PRIVILEGE_DRIVER_LEVEL_2,
    PRIVILEGE_USER_LEVEL,
};

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define SEGMENTS_COUNT 6

enum GDT_ENTRY_INDEX {
    GDT_INDEX_NULL_SEGMENT = 0,
    GDT_INDEX_KERNEL_CODE_SEGMENT,
    GDT_INDEX_KERNEL_DATA_SEGMENT,
    GDT_INDEX_USERLAND_CODE_SEGMENT,
    GDT_INDEX_USERLAND_DATA_SEGMENT,
    GDT_INDEX_TSS,
};

#define OPERATING_USING_16BITS 0
#define OPERATING_USING_32BITS 1

#define SYSTEM_DESCRIPTOR 0
#define CODE_OR_DATA_DESCRIPTOR 1

// Data descriptors flags
#define READ_FLAG               0b0000
#define ACCESS_FLAG             0b0001
#define WRITE_FLAG              0b0010
#define EXPAND_DOWN_FLAG        0b0100

// Code descriptors flags
#define EXECUTE_FLAG            0b1000
#define CONFORMING_FLAG         0b0100

#define GRANULARITY_IN_BYTES 0
#define GRANULARITY_IN_PAGES 1

struct SegmentSelector {
    u16 privilege_level: 1;
    u16 table_indicator: 1; // 0 -> GDT, 1 -> LDT
    u16 index: 14;
};

struct SegmentDescriptor {
    u16 segment_limit;
    u32 base_address: 24;
    u32 segment_type: 4;
    u32 descriptor_type: 1;
    u32 descriptor_privilege_level: 2;
    u32 is_segment_present: 1;
    u8 segment_limit_extension: 4;
    u8 avl: 1;
    u8 extended_code_segment: 1;
    u8 operating_size: 1;
    u8 granularity: 1;
    u8 base_address_extension;
}__attribute__((packed));

struct GlobalDescriptorTable {
    struct SegmentDescriptor descriptors[SEGMENTS_COUNT];
};

static struct GlobalDescriptorTable GDT;

struct GDTR {
    u16 limit;
    struct GlobalDescriptorTable* base;
}__attribute__((packed));

void enable_protected_mode() {
    asm volatile("movl %cr0, %eax\nor $1, %eax\nmovl %cr0, %eax\n");
}

void print_segment_descriptor(struct SegmentDescriptor *segment) {
    printf("\tSegment starts at : \t0x%x\n", segment->base_address | (segment->base_address_extension >> 16));
    printf("\tSegment size is : \t0x%x\n", segment->segment_limit | (segment->segment_limit_extension >> 16));

    if (segment->is_segment_present) {
        printf("\tSegment is present : \tyes\n");
    } else {
        printf("\tSegment is present : \tno\n");
    }

    printf("\tSegment type is : \t");
    if (segment->segment_type & READ_FLAG) {
        printf("READ ");
    }
    if (segment->segment_type & EXECUTE_FLAG) {
        printf("EXECUTE ");
    }
    if (segment->segment_type & WRITE_FLAG) {
        printf("WRITE ");
    }
    printf("\n");

    if (segment->descriptor_privilege_level == PRIVILEGE_USER_LEVEL) {
        printf("\tSegment privilege level : \tUSER\n");
    }
    if (segment->descriptor_privilege_level == PRIVILEGE_KERNEL_LEVEL) {
        printf("\tSegment privilege level : \tKERNEL\n");
    }

    if (segment->extended_code_segment) {
        printf("\tSegment mode : \t64bit\n");
    } else {
        printf("\tSegment mode : \t32bit\n");
    }

    if (segment->avl) {
        printf("\tSegment usable by system software : \tyes\n");
    } else {
        printf("\tSegment usable by system software : \tno\n");
    }

    if (segment->operating_size) {
        printf("\tSegment operation size : \t32 bit\n");
    } else {
        printf("\tSegment operation size : \t16bit\n");
    }

    if (segment->granularity == GRANULARITY_IN_BYTES) {
        printf("\tSegment granularity : \tbytes\n");
    } else {
        printf("\tSegment granularity : \tpages\n");
    }
}

void print_GDT() {
    printf("%s", "GDT layout : --------------------\n");
    printf("%s", "Kernel code descriptor : \n");
    print_segment_descriptor(&GDT.descriptors[GDT_INDEX_KERNEL_CODE_SEGMENT]);
    printf("%s", "Kernel data descriptor : \n");
    print_segment_descriptor(&GDT.descriptors[GDT_INDEX_KERNEL_DATA_SEGMENT]);
    printf("%s", "User code descriptor : \n");
    print_segment_descriptor(&GDT.descriptors[GDT_INDEX_USERLAND_CODE_SEGMENT]);
    printf("%s", "User data descriptor : \n");
    print_segment_descriptor(&GDT.descriptors[GDT_INDEX_USERLAND_DATA_SEGMENT]);
}

void set_GDT() {
    struct GDTR gdtr;
    gdtr.base = &GDT;
    gdtr.limit = sizeof(struct GlobalDescriptorTable) - 1;

    asm volatile("lgdt %0\n" : : "m" (gdtr) : "memory");
}

void init_GDT() {
    struct SegmentDescriptor null_descriptor = {0};
    struct SegmentDescriptor kernel_code;
    kernel_code.segment_limit = 0xffff;
    kernel_code.base_address = 0x0;
    kernel_code.segment_type = EXECUTE_FLAG | READ_FLAG;
    kernel_code.descriptor_type = CODE_OR_DATA_DESCRIPTOR;
    kernel_code.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    kernel_code.is_segment_present = 1;
    kernel_code.segment_limit_extension = 0xf;
    kernel_code.avl = 0;
    kernel_code.extended_code_segment = 0;
    kernel_code.operating_size = 1;
    kernel_code.granularity = GRANULARITY_IN_PAGES;
    kernel_code.base_address_extension = 0;
    struct SegmentDescriptor kernel_data;
    kernel_data.segment_limit = 0xffff;
    kernel_data.base_address = 0x0;
    kernel_data.segment_type = READ_FLAG | WRITE_FLAG;
    kernel_data.descriptor_type = CODE_OR_DATA_DESCRIPTOR;
    kernel_data.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    kernel_data.is_segment_present = 1;
    kernel_data.segment_limit_extension = 0xf;
    kernel_data.avl = 0;
    kernel_data.extended_code_segment = 0;
    kernel_data.operating_size = 1;
    kernel_data.granularity = GRANULARITY_IN_PAGES;
    kernel_data.base_address_extension = 0;
    struct SegmentDescriptor user_code;
    user_code.segment_limit = 0xffff;
    user_code.base_address = 0x0;
    user_code.segment_type = EXECUTE_FLAG | READ_FLAG;
    user_code.descriptor_type = CODE_OR_DATA_DESCRIPTOR;
    user_code.descriptor_privilege_level = PRIVILEGE_USER_LEVEL;
    user_code.is_segment_present = 1;
    user_code.segment_limit_extension = 0xf;
    user_code.avl = 0;
    user_code.extended_code_segment = 0;
    user_code.operating_size = 1;
    user_code.granularity = GRANULARITY_IN_PAGES;
    user_code.base_address_extension = 0;
    struct SegmentDescriptor user_data;
    user_data.segment_limit = 0xffff;
    user_data.base_address = 0x0;
    user_data.segment_type = READ_FLAG | WRITE_FLAG;
    user_data.descriptor_type = CODE_OR_DATA_DESCRIPTOR;
    user_data.descriptor_privilege_level = PRIVILEGE_USER_LEVEL;
    user_data.is_segment_present = 1;
    user_data.segment_limit_extension = 0xf;
    user_data.avl = 0;
    user_data.extended_code_segment = 0;
    user_data.operating_size = 1;
    user_data.granularity = GRANULARITY_IN_PAGES;
    user_data.base_address_extension = 0;
    struct SegmentDescriptor tss_descriptor = {0};

    // Update the entries
    GDT.descriptors[GDT_INDEX_NULL_SEGMENT] = null_descriptor;
    GDT.descriptors[GDT_INDEX_KERNEL_CODE_SEGMENT] = kernel_code;
    GDT.descriptors[GDT_INDEX_KERNEL_DATA_SEGMENT] = kernel_data;
    GDT.descriptors[GDT_INDEX_USERLAND_CODE_SEGMENT] = user_code;
    GDT.descriptors[GDT_INDEX_USERLAND_DATA_SEGMENT] = user_data;
    GDT.descriptors[GDT_INDEX_TSS] = tss_descriptor;

    print_GDT();

    set_GDT();

    enable_protected_mode();

    // set segments
    asm volatile("pushl $0x08\npushl $1f\nlret\n1:"); // seek label 1: forward

    asm volatile("movw $0x10, %ax");
    asm volatile("movw %ax, %ds\n");
    asm volatile("movw %ax, %es\n");
    asm volatile("movw %ax, %fs\n");
    asm volatile("movw %ax, %gs\n");
    asm volatile("movw %ax, %ss\n");
}

// !GDT related stuff --------------------------------------

// !IDT related stuff --------------------------------------
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

#define INTERRUPTS_COUNT 255
#define INTERRUPT_GATE 0b110
#define TRAP_GATE 0b111

struct InterruptDescriptorTable {
    struct Gate descriptors[INTERRUPTS_COUNT];
};

struct IDTR {
    u16 limit;
    struct InterruptDescriptorTable* base;
}__attribute__((packed));

static struct InterruptDescriptorTable IDT;

void set_IDT() {
    struct IDTR idtr;
    idtr.base = &IDT;
    idtr.limit = sizeof(struct InterruptDescriptorTable) - 1;

    asm volatile("lidt %0\n" : : "m" (idtr) : "memory");
}

void set_interrupt_gate_IDT(u8 gate_index, void* offset) {
    struct Gate interrupt_gate = { 0 };
    interrupt_gate.offset = (u16)offset;
    interrupt_gate.segment_selector = KERNEL_CODE_SELECTOR;
    interrupt_gate.gate_type = INTERRUPT_GATE;
    interrupt_gate.gate_size = 1;
    interrupt_gate.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    interrupt_gate.segment_present = 1;
    interrupt_gate.extended_offset = (u16)((u32)offset >> 16);

    IDT.descriptors[gate_index] = interrupt_gate;
}

void set_trap_gate_IDT(u8 gate_index, void* offset) {
    struct Gate trap_gate = { 0 };
    trap_gate.offset = (u16)offset;
    trap_gate.segment_selector = KERNEL_CODE_SELECTOR;
    trap_gate.gate_type = TRAP_GATE;
    trap_gate.gate_size = 1;
    trap_gate.descriptor_privilege_level = PRIVILEGE_KERNEL_LEVEL;
    trap_gate.segment_present = 1;
    trap_gate.extended_offset = (u16)((u32)offset >> 16);

    IDT.descriptors[gate_index] = trap_gate;
}

void init_IDT() {
    set_interrupt_gate_IDT(3, (void*) &isr_3);
    set_interrupt_gate_IDT(8, (void*) &isr_8);
    set_interrupt_gate_IDT(10, (void*) &isr_10);
    set_interrupt_gate_IDT(17, (void*) &isr_17);

    set_IDT();
}

// !IDT related stuff --------------------------------------

void k_main(unsigned long magic, multiboot_info_t *info)
{
    (void)magic;
    (void)info;

    char star[4] = "|/-\\";
    char *fb = (void *)0xb8000;

    init_COM1();
    init_GDT();
    init_IDT();

    // asm volatile ("int $3");
    asm volatile ("int $10");

    for (unsigned i = 0; ; ) {
        *fb = star[i++ % 4];
    }

    for (;;)
        asm volatile ("hlt");
}
