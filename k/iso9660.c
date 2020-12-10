#include <k/iso9660.h>
#include <k/types.h>
#include <k/atapi.h>

static struct iso_prim_voldesc PRIMARY_VOLUME = { '\0' };
static struct ise_path_table_le PATH_TABLE = { '\0' };

void read_primary_volume_descriptor(void) {
    read_block(ISO_PRIM_VOLDESC_BLOCK, (u16*) &PRIMARY_VOLUME);

    read_block(PRIMARY_VOLUME.le_path_table_blk, (u16*) &PATH_TABLE);
}

// Filesystem interface
int open(const char *pathname, int flags) {
    // Recurse to pathname
    // Register a file descriptor (index) and struct (directory extent + flags)
    // return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    // Translate count into a sector_number
    // Check flags for the fd and read sector_number sector
    // Copy to buf
    // return count;
    // return -1; if something went wrong
}

size_t seek(int fd, size_t offset, int whence);

int close(int fd) {
    // Remove the fd entry from fd array
}
