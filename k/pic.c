#include <k/pic.h>

#include <k/io.h>

void init_PIC() {
    // Initialize the master PIC with ICWs
    outb(MASTER_PIC_COMMAND_PORT, ICW1_BASE | ICW1_ENABLE_ICW4);
    io_wait();
    outb(MASTER_PIC_DATA_PORT, MASTER_PIC_VECTOR_OFFSET);
    io_wait();
    outb(MASTER_PIC_DATA_PORT, MASTER_PIC_WIRING);
    io_wait();
    outb(MASTER_PIC_DATA_PORT, ICW4_BASE);
    io_wait();

    outb(SLAVE_PIC_COMMAND_PORT, ICW1_BASE | ICW1_ENABLE_ICW4);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, MASTER_PIC_VECTOR_OFFSET);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, SLAVE_PIC_WIRING);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, ICW4_BASE);
    io_wait();


    // Mask all interrupts through OCW1 to PIC's data port
    outb(MASTER_PIC_DATA_PORT, 0b11111111);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, 0b11111111);
    io_wait();
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

void acknowledge_latest_master_IRQ() {
    outb(MASTER_PIC_COMMAND_PORT, OCW2_ACKNOWLEDGE_IRQ);
}
void acknowledge_latest_slave_IRQ() {
    acknowledge_latest_master_IRQ();
    outb(SLAVE_PIC_COMMAND_PORT, OCW2_ACKNOWLEDGE_IRQ);
}
