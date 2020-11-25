#ifndef K_PIC_H
#define K_PIC_H

#include <k/types.h>

#define MASTER_PIC_COMMAND_PORT     0x20
#define MASTER_PIC_DATA_PORT        0x21

#define SLAVE_PIC_COMMAND_PORT      0xA0
#define SLAVE_PIC_DATA_PORT         0xA1

// ICW1
#define ICW1_ENABLE_ICW4            0b00000001
#define ICW1_SINGLE_PIC_MODE        0b00000010
#define ICW1_LEVEL_TRIGGERED_MODE   0b00001000

// No ICW4, cascade mode and edge triggered mode
#define ICW1_BASE                   0b00010000

// ICW2
#define MASTER_PIC_VECTOR_OFFSET    0x20
#define SLAVE_PIC_VECTOR_OFFSET     0x28

// ICW3
// Define which IRQ is connected to the slave PIC
#define MASTER_PIC_WIRING           0b00000100
// Define which IRQ of the master is connected to the slave
#define SLAVE_PIC_WIRING            0b00000010

// ICW4
#define ICW4_AUTOMATIC_EOI          0b00000010

// No buffering, no special mode and no automatic EOI
#define ICW4_BASE                   0b00000001

#define OCW2_EOI                    0b00100000

void init_PIC();

void mask_IRQ(u8 IRQ_index);
void unmask_IRQ(u8 IRQ_index);

void send_EOI(u8 latest_IRQ_index);

#endif				/* !PIC_H_ */
