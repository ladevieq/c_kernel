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
s32 insert_file(struct File* file) {
    s32 index = 0;

    while(FD_TABLE[index].size != 0) {
    }

    if (index < FD_TABLE_SIZE) {
        FD_TABLE[index] = *file;
        return index;
    }

    return -1;
}

s32 remove_file(s32 fd) {
    if (FD_TABLE[fd].size != 0) {
        FD_TABLE[fd].size = 0;
        return 0;
    }

    return -1;
}

struct File* get_file(s32 fd) {
    return &FD_TABLE[fd];
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

// -------------------------------------
// ISO API
// -------------------------------------

s32 is_valid_path(const char* path) {
    if (path[0] != '/') {
        return 0;
    }

    return 1;
}

// Find first occurence of a char in string starting from offset
// return char position in string
s32 find_first(const char* string, char seeked_char, off_t offset) {
    size_t len = strlen(string);
    for(;string[offset] != seeked_char && (size_t)offset < len; offset++) {
    }

    if (string[offset] == seeked_char) {
        return offset;
    }

    return -1;
}

// Find a file or dir in a dir
s32 find_in_dir(struct iso_dir* dir, const char* filename, size_t len, struct iso_dir* seeked_dir) {
    if (dir->type != ISO_FILE_ISDIR) {
        return -1;
    }

    // Read dir first entry
    u8 block[CD_BLOCK_SZ] = { '\0' };
    read_block(dir->data_blk.le, (void*) &block);

    for(u8*dir_ptr = &block[0];
        ((struct iso_dir*)dir_ptr)->dir_size != 0;
        dir_ptr += ((struct iso_dir*)dir_ptr)->dir_size) {
        struct iso_dir* cur_dir = (struct iso_dir*) dir_ptr;

        if (strncasecmp(filename, cur_dir->idf, len) == 0) {
            *seeked_dir = *cur_dir;
            return 0;
        }
    }
    
    return -1;
}

s32 find(const char* path, struct iso_dir* seeked_dir) {
    struct iso_dir current_dir = PRIMARY_VOLUME.root_dir;
    off_t dirname_offset = 1;
    u32 dirname_start = dirname_offset;
    size_t len = 0;


    if (!is_valid_path(path)) {
        return -1;
    }

    while((dirname_offset = find_first(path, '/', dirname_offset)) != -1) {
        len = dirname_offset - dirname_start;

        if (find_in_dir(&current_dir, &path[dirname_start], len, &current_dir) == -1) {
            return -1;
        }

        // Skip '/'
        dirname_start = ++dirname_offset;
    }


    len = strlen(path) - dirname_start;
    if (find_in_dir(&current_dir, &path[dirname_start], len, seeked_dir) == -1) {
        return -1;
    }

    return 0;
}

s32 find_file(const char* path, struct File* file) {
    struct iso_dir dir = {};
    if (find(path, &dir) == -1) {
        return -1;
    }

    file->initial_lba = dir.data_blk.le;
    file->offset = 0;
    file->size = dir.file_size.le;

    return 0;
}
