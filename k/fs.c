#include <k/fs.h>
#include <k/iso9660.h>
#include <k/atapi.h>

#include <string.h>

// Filesystem interface
int open(const char *pathname, int flags) {
    struct File file = {};

    if (flags != O_RDONLY) {
        return -1;
    }

    if (find_file(pathname, &file) == -1) {
        return -1;
    }

    return insert_file(&file);
}

ssize_t read(int fd, void *buf, size_t count) {
    u8 block[CD_BLOCK_SZ] = { '\0' };
    struct File* file = get_file(fd);
    u32 sector_len = (((count < file->size) ? count : file->size) / CD_BLOCK_SZ) + 1;
    u32 start_lba = file->initial_lba + (file->offset / CD_BLOCK_SZ);
    ssize_t read_count = 0;
    off_t buf_offset = 0;

    for(u32 current_lba = start_lba; current_lba < start_lba + sector_len; current_lba++) {
        read_block(current_lba, &block);

        // We start to copy the data from block + file_off
        off_t block_offset = file->offset % CD_BLOCK_SZ;
        // Assume we'll read till the end of the block
        size_t cpy_len = CD_BLOCK_SZ - block_offset;
        // Check if the file stop before block end
        if (file->size - block_offset < cpy_len) {
            cpy_len = file->size - block_offset;
        }
        // Check if the amount of data to read left is smaller
        if (read_count + cpy_len > count) {
            cpy_len = count - read_count;
        }

        memcpy(buf + buf_offset, block + block_offset, cpy_len);

        buf_offset += cpy_len;
        read_count += cpy_len;
        file->offset += cpy_len;
    }

    return read_count;
}

off_t seek(int fd, off_t offset, int whence) {
    struct File* file = get_file(fd);

    switch(whence) {
        case SEEK_SET: {
            file->offset = offset;
            break;
        }
        case SEEK_CUR: {
            file->offset += offset;
            break;
        }
        case SEEK_END: {
            file->offset = file->size + offset;
            break;
        }
    }

    return file->offset;
}

int close(int fd) {
    return remove_file(fd);
}
