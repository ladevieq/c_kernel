#include <k/elf.h>
#include <k/iso9660.h>

#include <stdio.h>
#include <string.h>

s32 load_ELF(const char* elf_path) {
    s32 fd = open(elf_path, O_RDONLY);
    if(fd != -1) {
        printf("Sucessfully opened directory %s\n", elf_path);
    }

    u8 magic[4] = { '\0' };
    read(fd, (void*) &magic, sizeof(magic));

    if (magic[0] != 0x7F || strncasecmp((const char*) &magic[1], "ELF", 3) != 0) {
        return -1;
    }

    seek(fd, SEEK_SET, 0);

    Elf_Ehdr elf_header = {};
    read(fd, (void*) &elf_header, sizeof(elf_header));
    seek(fd, SEEK_SET, elf_header.e_phoff);

    size_t ph_entry_size = elf_header.e_phentsize;
    for (s32 ph_index = 0; ph_index < elf_header.e_phnum; ph_index++) {
        Elf_Phdr program_header = {};

        read(fd, (void*) &program_header, ph_entry_size);

        if (program_header.p_filesz > 0) {
            if (program_header.p_filesz != program_header.p_memsz) {
                size_t bytes_to_clear = program_header.p_memsz - program_header.p_filesz;
                memset((void*) (program_header.p_vaddr + program_header.p_filesz), 0, bytes_to_clear);
            }

            // read segment to memory
            // memcpy(program_header.p_vaddr, , program_header.p_filesz);
        }
    }

    void (*entry_point)(void) = (void*)elf_header.e_entry;

    entry_point();

    return 0;
}
