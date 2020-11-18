#include <k/gdt.h>

#include <stdio.h>

static struct GlobalDescriptorTable GDT;

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
