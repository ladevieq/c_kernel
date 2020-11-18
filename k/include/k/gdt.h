#ifndef K_GDT_H
#define K_GDT_H

#include <k/types.h>

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

struct GDTR {
    u16 limit;
    struct GlobalDescriptorTable* base;
}__attribute__((packed));


void init_GDT();

void print_GDT();

#endif				/* !GDT_H_ */
