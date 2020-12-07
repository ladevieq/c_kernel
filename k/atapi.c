#include <k/atapi.h>
#include <k/io.h>

#include <string.h>
#include <stdio.h>

// Methodology
// Write ‘waiting’ helper functions:
// void busy_wait(u16 drive);
// void wait_device_selection(u16 drive);
// void wait_packet_request(u16 drive);
// These functions should only be doing inb() calls and status checking.
//
// Write functions to read data on the drive:
// int send_packet(struct SCSI_packet *pkt, u16 drive,u16 size);
// void *read_block(size_t lba);
// Feel free to modify the proposed function prototypes.

void init_ATAPI() {
    discover_atapi_drive();
}

// Wait helper functions
/* ATA specifies a 400ns delay after drive switching -- often
 * implemented as 4 Alternative Status queries. */
void wait_device_selection(u16 bus) {
    inb(bus);
    inb(bus);
    inb(bus);
    inb(bus);
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

    drive_ATAPI_signature[0] = inb(ATA_REG_SECTOR_COUNT(drive));
    drive_ATAPI_signature[1] = inb(ATA_REG_LBA_LO(drive));
    drive_ATAPI_signature[2] = inb(ATA_REG_LBA_MI(drive));
    drive_ATAPI_signature[3] = inb(ATA_REG_LBA_HI(drive));

    return memcmp(drive_ATAPI_signature, ATAPI_signature , 4) == 0;
}

void discover_atapi_drive() {
    outb(PRIMARY_DCR, SRST);
    outb(PRIMARY_DCR, INTERRUPT_DISABLE);

    outb(SECONDARY_DCR, SRST);
    outb(SECONDARY_DCR, INTERRUPT_DISABLE);

    u16 ATA_buses[2] = { PRIMARY_REG, SECONDARY_REG };
    u8 drives[2]  = { ATA_PORT_MASTER, ATA_PORT_SLAVE };

    for (size_t bus_index; bus_index < 2; bus_index++) {
        u16 bus = ATA_buses[bus_index];

        for (size_t drive_index; drive_index < 2; drive_index++) {
            u8 drive = drives[drive_index];

            select_drive            (bus, drive);
            wait_device_selection   (bus);
            if (is_atapi_drive          (bus, drive)) {
                printf ("Bus : %d\t Drive : %d\t is an ATAPI drive\n", bus, drive);
            }
        }
    }
}
