#include <k/elf.h>
#include <k/iso9660.h>
#include <k/atapi.h>

#include <stdio.h>
#include <string.h>

void print_flags(Elf32_Word flags) {
    printf("Flags : \t");

    if (flags & PF_R) {
        printf("R ");
    }
    if (flags & PF_W) {
        printf("W ");
    } 
    if (flags & PF_X) {
        printf("X ");
    }

    printf("\n");
}

void print_program_header(Elf32_Phdr* program_header) {
    switch(program_header->p_type) {
        case PT_NULL: {
            printf("UNUSED segment : -------------\n");
            break;
        }
        case PT_LOAD: {
            printf("LOAD segment : -------------\n");
            break;
        }
        case PT_DYNAMIC: {
            printf("DYNAMIC segment : -------------\n");
            break;
        }
        case PT_INTERP: {
            printf("INTERP segment : -------------\n");
            break;
        }
        case PT_NOTE: {
            printf("NOTE segment : -------------\n");
            break;
        }
    }

    printf("Offset : \t%#010x\n", program_header->p_offset);
    printf("VirtAddr : \t%#010x\n", program_header->p_vaddr);
    printf("PhysAddr : \t%#010x\n", program_header->p_paddr);
    printf("FileSz : \t%#010x\n", program_header->p_filesz);
    printf("MemSz : \t%#010x\n", program_header->p_memsz);
    print_flags(program_header->p_flags);
    printf("\n");
}

void pad_segment(Elf32_Phdr* program_header) {
    size_t padding_len = program_header->p_memsz - program_header->p_filesz;
    memset((void*) (program_header->p_vaddr + program_header->p_filesz), 0, padding_len);
}

void load_segment(Elf32_Phdr* program_header, u32 elf_first_block) {
    if (program_header->p_filesz > 0) {
        u8 block[2048] = { 0 };
        size_t block_count = (program_header->p_filesz / CD_BLOCK_SZ) | 1;
        off_t first_block = elf_first_block + (program_header->p_offset / CD_BLOCK_SZ);
        off_t dest_off = 0;

        for (u32 block_index = 0; block_index < block_count; block_index++) {
            size_t cpy_len = CD_BLOCK_SZ;
            off_t src_off = 0;

            if (block_index == 0) {
                // Crop the start of the block
                cpy_len -= program_header->p_offset % CD_BLOCK_SZ;
                src_off = program_header->p_offset;
            } else if (block_index == block_count) {
                // Crop the end of the block
                cpy_len -= CD_BLOCK_SZ - (program_header->p_filesz % CD_BLOCK_SZ);
            }

            read_block(first_block + block_index, &block[0]);

            memcpy((void*) program_header->p_vaddr + dest_off, &block[src_off], cpy_len);

            dest_off += cpy_len;
        }
    }

    if (program_header->p_filesz != program_header->p_memsz) {
        pad_segment(program_header);
    }
}

s32 load_ELF(const char* elf_path) {
    u8 block [CD_BLOCK_SZ] = { 0 };
    struct File elf_file = {};

    if (find_file(elf_path, &elf_file) != -1) {
        printf("Sucessfully opened directory %s\n", elf_path);
    }

    u32 current_block = elf_file.initial_lba; 
    read_block(elf_file.initial_lba, &block[0]);

    u8 magic[4] = { block[0], block[1], block[2], block[3] };
    if (magic[0] != 0x7F || strncasecmp((const char*) &magic[1], "ELF", 3) != 0) {
        return -1;
    }

    Elf_Ehdr elf_header = {};
    memcpy(&elf_header, block, sizeof(elf_header));

    for (s32 ph_index = 0; ph_index < elf_header.e_phnum; ph_index++) {
        Elf_Phdr program_header = {};
        off_t cur_header_off = elf_header.e_phoff + (ph_index * elf_header.e_phentsize);
        size_t ph_off_block = cur_header_off / CD_BLOCK_SZ;
        off_t cpy_off = cur_header_off % CD_BLOCK_SZ;

        if (current_block != elf_file.initial_lba + ph_off_block) {
            read_block(elf_file.initial_lba + ph_off_block, &block[0]);
        }

        memcpy(&program_header, &block[cpy_off], sizeof(program_header));

        print_program_header(&program_header);

        if (program_header.p_type == PT_LOAD) {
            load_segment(&program_header, elf_file.initial_lba);
        }
    }

    void (*entry_point)(void) = (void*)elf_header.e_entry;

    entry_point();

    return 0;
}
