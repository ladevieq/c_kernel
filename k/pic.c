#include <k/pic.h>

#include <k/io.h>

void init_PIC() {
    // Initialize the PICs with ICWs
    // ICW1 PIC's mode of operation
    outb(MASTER_PIC_COMMAND_PORT, ICW1_BASE | ICW1_ENABLE_ICW4);
    outb(SLAVE_PIC_COMMAND_PORT, ICW1_BASE | ICW1_ENABLE_ICW4);

    // ICW2 PIC's interrupts vector offest
    outb(MASTER_PIC_DATA_PORT, MASTER_PIC_VECTOR_OFFSET);
    outb(SLAVE_PIC_DATA_PORT, SLAVE_PIC_VECTOR_OFFSET);

    // ICW3 how master and slave PIC's are connected
    outb(MASTER_PIC_DATA_PORT, MASTER_PIC_WIRING);
    outb(SLAVE_PIC_DATA_PORT, SLAVE_PIC_WIRING);

    // ICW4 additional operation modes
    outb(MASTER_PIC_DATA_PORT, ICW4_BASE);
    outb(SLAVE_PIC_DATA_PORT, ICW4_BASE);


    // Mask all interrupts through OCW1 to PIC's data port
    outb(MASTER_PIC_DATA_PORT, 0b11111100);
    outb(SLAVE_PIC_DATA_PORT, 0b11111111);

    enable_PIC_interrupts();
}

void mask_IRQ(u8 IRQ_index) {
    u8 active_mask = 0;

    if (IRQ_index >= 8) {
        active_mask = inb(SLAVE_PIC_DATA_PORT);
        active_mask |= IRQ_index >> 8;
        outb(SLAVE_PIC_DATA_PORT, active_mask);
    } else {
        active_mask = inb(MASTER_PIC_DATA_PORT);
        active_mask |= IRQ_index;
        outb(MASTER_PIC_DATA_PORT, active_mask);
    }
}

void unmask_IRQ(u8 IRQ_index) {
    u8 active_mask = 0;

    if (IRQ_index >= 8) {
        active_mask = inb(SLAVE_PIC_DATA_PORT);
        active_mask &= (0xff ^ (IRQ_index >> 8));
        outb(SLAVE_PIC_DATA_PORT, active_mask);
    } else {
        active_mask = inb(MASTER_PIC_DATA_PORT);
        active_mask &= (0xff ^ IRQ_index);
        outb(MASTER_PIC_DATA_PORT, active_mask);
    }
}

void send_EOI(u8 vector_number) {
    outb(MASTER_PIC_COMMAND_PORT, OCW2_EOI);

    if (vector_number >= SLAVE_PIC_VECTOR_OFFSET) {
        outb(SLAVE_PIC_COMMAND_PORT, OCW2_EOI);
    }
}

void enable_PIC_interrupts() {
    asm volatile("sti");
}

void disable_PIC_interrupts() {
    asm volatile("cli");
}
