#include <k/iso9660.h>
#include <k/types.h>
#include <k/atapi.h>

#include <stdio.h>
#include <string.h>

static struct iso_prim_voldesc PRIMARY_VOLUME = { 0 };

static struct File FD_TABLE[FD_TABLE_SIZE] = {{ 0 }};


// --------------------------------------
// File descriptor management functions
// --------------------------------------
s32 insert_dir(struct File* file) {
    s32 index = 0;

    while(FD_TABLE[index].size != 0) {
    }

    if (index < FD_TABLE_SIZE) {
        FD_TABLE[index] = *file;
        return index;
    }

    return -1;
}

s32 remove_dir(s32 fd) {
    if (FD_TABLE[fd].size != 0) {
        FD_TABLE[fd].size = 0;
        return 0;
    }

    return -1;
}

// --------------------------------------
// ! File descriptor management functions
// --------------------------------------


void init_ISO(void) {
    read_block(ISO_PRIM_VOLDESC_BLOCK, &PRIMARY_VOLUME);
}


// -------------------------------------
// ISO structures pretty printers
// -------------------------------------
void print_filename(struct iso_dir* dir) {
    if (dir->idf_len == 1) {
        if (dir->idf[0] == '\0') {
            printf("Name . :\n", dir->idf_len, dir->idf);
        } else if (dir->idf[0] == '\1') {
            printf("Name .. :\n", dir->idf_len, dir->idf);
        }
    } else {
        u32 filename_real_length = 0;
        char* cur_char = &dir->idf[0];
        while(*(cur_char++) != ';' && filename_real_length < dir->idf_len) {
            filename_real_length++;
        }
        printf("Name %.*s :\n", filename_real_length, dir->idf);
    }
}

void print_filetype(struct iso_dir* dir) {
    switch(dir->type) {
        case ISO_FILE_HIDDEN: {
            printf("\tFile type : Hidden \n", dir->file_size.le);
            break;
        }
        case ISO_FILE_ISDIR: {
            printf("\tFile type : Is Dir\n", dir->file_size.le);
            break;
        }
        case ISO_FILE_ASSOCIAT: {
            printf("\tFile type : Associated\n", dir->file_size.le);
            break;
        }
        case ISO_FILE_USEEXT: {
            printf("\tFile type : Use Ext\n", dir->file_size.le);
            break;
        }
        case ISO_FILE_USEPERM: {
            printf("\tFile type : Use Perm\n", dir->file_size.le);
            break;
        }
        case ISO_FILE_MULTIDIR: {
            printf("\tFile type : Multi Dir\n", dir->file_size.le);
            break;
        }
    }
}

void print_dir(struct iso_dir* dir) {
    print_filename(dir);
    printf("\tLBA %u\n", dir->data_blk.le);
    printf("\tFile size %u\n", dir->file_size.le);
    print_filetype(dir);
}

void print_dir_entries(struct iso_dir* dir) {
    u8 block[CD_BLOCK_SZ] = { '\0' };
    u8* dir_ptr = &block[0];

    print_dir(dir);

    read_block(dir->data_blk.le, (u16*) &block);

    while(((struct iso_dir*)dir_ptr)->dir_size != 0) {
        struct iso_dir* cur_dir = (struct iso_dir*) dir_ptr;
        print_dir(cur_dir);

        dir_ptr += cur_dir->dir_size;
    }
}
// -------------------------------------
// ! ISO structures pretty printers
// -------------------------------------

// Find a filename in a dir
int find_in_dir(const char* filename, size_t len, struct iso_dir* dir) {
    if (dir->type != ISO_FILE_ISDIR) {
        return -1;
    }

    u8 block[CD_BLOCK_SZ] = { '\0' };
    u8* dir_ptr = &block[0];

    read_block(dir->data_blk.le, (u16*) &block);

    while(((struct iso_dir*)dir_ptr)->dir_size != 0) {
        struct iso_dir* cur_dir = (struct iso_dir*) dir_ptr;

        if (strncmp(filename, cur_dir->idf, len) == 0) {
            *dir = *cur_dir;
            return 0;
        }

        dir_ptr += cur_dir->dir_size;
    }
    
    return -1;
}

// Filesystem interface
int open(const char *pathname, int flags) {
    if (flags != O_RDONLY || pathname[0] != '/') {
        return -1;
    }

    size_t path_len = strlen(pathname);
    struct iso_dir* current_dir = &PRIMARY_VOLUME.root_dir;
    u32 current_char = 0;

    while(current_char < path_len) {
        if (pathname[current_char] == '/') {
            current_char++;
        }

        size_t filename_len = 0;
        const char* filename_start = &pathname[current_char];
        const char* pathname_iter = &pathname[current_char];

        while(*pathname_iter != '\0' && *pathname_iter != '/') {
            (pathname_iter++);
            filename_len++;
        }

        if (find_in_dir(filename_start, filename_len, current_dir) == -1) {
            return -1;
        }

        print_dir_entries(current_dir);

        current_char += filename_len;
    }

    struct File file = {
        .initial_lba = current_dir->data_blk.le,
        .offset = 0,
        .size = current_dir->file_size.le,
    };

    return insert_dir(&file);
}

ssize_t read(int fd, void *buf, size_t count) {
    u8 block[CD_BLOCK_SZ] = { '\0' };
    u32 sector_number = (count / CD_BLOCK_SZ < 1) ? 1 : count / CD_BLOCK_SZ;
    struct File* file = &FD_TABLE[fd];
    u32 start_lba = file->initial_lba + (file->offset / CD_BLOCK_SZ);
    ssize_t read_count = 0;
    off_t buf_offset = 0;

    for(u32 current_lba = start_lba; current_lba < start_lba + sector_number; current_lba++) {
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
    struct File* file = &FD_TABLE[fd];

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
    return remove_dir(fd);
}
