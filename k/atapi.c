#include <k/atapi.h>
#include <k/io.h>

#include <string.h>
#include <stdio.h>

// #define DEBUG_ATAPI

static u16 ATAPI_bus = 0;
static u8 ATAPI_drive = 0;

// Wait helper functions
void busy_wait(u16 bus) {
    while((inb(ATA_REG_STATUS(bus)) & BSY) != 0) {
#ifdef DEBUG_ATAPI
        printf("Busy waiting for the disk\n");
#endif
    }
}

/* ATA specifies a 400ns delay after drive switching -- often
 * implemented as 4 Alternative Status queries. */
void wait_device_selection(u16 bus) {
    inb(bus);
    inb(bus);
    inb(bus);
    inb(bus);
}

void wait_packet_requested(u16 bus) {
    while((inb(ATA_REG_STATUS(bus)) & BSY) == BSY ||
          (inb(ATA_REG_STATUS(bus)) & DRQ) != DRQ) {
#ifdef DEBUG_ATAPI
        printf("Waiting for the disk to request a packet\n");
#endif
    }
}


// Devices discovery functions
void select_drive(u16 bus, u8 drive) {
    outb(ATA_REG_DRIVE(bus), drive);
}

s32 is_atapi_drive(u16 bus, u8 drive) {
    u8 ATAPI_signature[4] = {
        ATAPI_SIG_SC,
        ATAPI_SIG_LBA_LO,
        ATAPI_SIG_LBA_MI,
        ATAPI_SIG_LBA_HI
    };
    u8 drive_ATAPI_signature[4] = { '\0' };

    select_drive            (bus, drive);
    wait_device_selection   (bus);

    drive_ATAPI_signature[0] = inb(ATA_REG_SECTOR_COUNT(bus));
    drive_ATAPI_signature[1] = inb(ATA_REG_LBA_LO(bus));
    drive_ATAPI_signature[2] = inb(ATA_REG_LBA_MI(bus));
    drive_ATAPI_signature[3] = inb(ATA_REG_LBA_HI(bus));

    return memcmp(drive_ATAPI_signature, ATAPI_signature , sizeof(ATAPI_signature)) == 0;
}

void discover_atapi_drive(void) {
    outb(PRIMARY_DCR, SRST);
    outb(PRIMARY_DCR, INTERRUPT_DISABLE);

    outb(SECONDARY_DCR, SRST);
    outb(SECONDARY_DCR, INTERRUPT_DISABLE);

    u16 ATA_buses[2] = { PRIMARY_REG, SECONDARY_REG };
    u8 drives[2]  = { ATA_PORT_MASTER, ATA_PORT_SLAVE };

    for (size_t bus_index = 0; bus_index < 2; bus_index++) {
        u16 bus = ATA_buses[bus_index];

        for (size_t drive_index = 0; drive_index < 2; drive_index++) {
            u8 drive = drives[drive_index];

            if (is_atapi_drive          (bus, drive)) {
                ATAPI_bus = bus;
                ATAPI_drive = drive;

#ifdef DEBUG_ATAPI
                if (bus == PRIMARY_REG) {
                    printf ("Primary bus\t Drive : %d\t is an ATAPI drive\n", drive);
                } else {
                    printf ("Secondary bus\t Drive : %d\t is an ATAPI drive\n", drive);
                }
#endif
            }
        }
    }
}


u32 send_packet(struct SCSI_packet *packet, u16 bus) {
    // Write the packet
    u16* raw_packet = (u16*)packet;
    for (size_t word_index = 0; word_index < (sizeof(struct SCSI_packet) / sizeof(u16)); word_index ++) {
        outw(ATA_REG_DATA(bus), raw_packet[word_index]);
    }

    while(inb(ATA_REG_SECTOR_COUNT(bus)) != PACKET_DATA_TRANSMIT) {
#ifdef DEBUG_ATAPI
        printf("Waiting for packet to be transmitted to the disk\n");
#endif
    }

    return 0;
}

void read_block(size_t lba, void* block) {
    struct SCSI_packet packet = {
        .op_code    = READ_12,
        .lba_hi     = lba >> 0x18,
        .lba_mihi   = lba >> 0x10,
        .lba_milo   = lba >> 0x08,
        .lba_lo     = lba,
        .transfer_length_lo = 1,
    };

    select_drive            (ATAPI_bus, ATAPI_drive);
    wait_device_selection   (ATAPI_bus);

    busy_wait               (ATAPI_bus);

    outb(ATA_REG_FEATURES(ATAPI_bus),       0);     // No overlap / DMA
    outb(ATA_REG_SECTOR_COUNT(ATAPI_bus),   0); // No queuing
    outb(ATA_REG_LBA_MI(ATAPI_bus),     (u8)CD_BLOCK_SZ);
    outb(ATA_REG_LBA_HI(ATAPI_bus),         CD_BLOCK_SZ >> 8);
    outb(ATA_REG_COMMAND(ATAPI_bus),        PACKET);   // Packet

    wait_packet_requested(ATAPI_bus);

    send_packet(&packet, ATAPI_bus);

    for (size_t word_index = 0; word_index < CD_BLOCK_SZ / sizeof(u16); word_index++) {
        ((u16*)block)[word_index] = inw(ATA_REG_DATA(ATAPI_bus));
    }
}

void init_ATAPI() {
    discover_atapi_drive();

    u8 block[CD_BLOCK_SZ];
    read_block(16, (u16*)block);
}

