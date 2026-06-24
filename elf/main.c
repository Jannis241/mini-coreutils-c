#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef unsigned char Byte;
const char *elf_type(uint16_t type) {
    switch (type) {
    case 0x0000:
        return "ET_NONE";
    case 0x0001:
        return "ET_REL";
    case 0x0002:
        return "ET_EXEC";
    case 0x0003:
        return "ET_DYN";
    case 0x0004:
        return "ET_CORE";
    default:
        return "UNKNOWN";
    }
}

const char *elf_machine(uint16_t machine) {
    switch (machine) {
    case 0x003e:
        return "x86-64";
    case 0x0003:
        return "x86";
    case 0x00b7:
        return "AArch64";
    case 0x0028:
        return "ARM";
    default:
        return "UNKNOWN";
    }
}

typedef struct {
    Byte magic_number[4];
    Byte bit_format;
    Byte endian;
    Byte version;
    Byte ABI;
    Byte ABI_specifics;
    Byte Padding[7];
    Byte Object_filetype[2];
    Byte TargetArchitecture[2];
    Byte ElfVersion[4];
    Byte ProgrammEntryPoint[8];      // für 64 bit
    Byte StartOfHeaderTable[8];      // für 64 bit
    Byte StartSectionHeaderTable[8]; // für 64 bit
    Byte flags[4];
    Byte header_size[2];
    Byte programm_header_table_entry_size[2];
    Byte number_program_header_table_entries[2];
    Byte header_section_size[2];
    Byte number_of_entries_section_header_table[2];
    Byte section_header_table_entry_which_contains_section_names_index[2];
} ELF_HEADER;

static uint16_t read_u16_le(Byte b[2]) {
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}

static uint32_t read_u32_le(Byte b[4]) {
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) |
           ((uint32_t)b[3] << 24);
}

static uint64_t read_u64_le(Byte b[8]) {
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--) {
        v = (v << 8) | b[i];
    }
    return v;
}

void print_elf_header(const ELF_HEADER *h) {
    printf("ELF Header\n");
    printf("==========\n");

    printf("Magic:                         ");
    for (int i = 0; i < 4; i++) {
        printf("%02x ", h->magic_number[i]);
    }
    printf("(%c%c%c)\n", h->magic_number[1], h->magic_number[2],
           h->magic_number[3]);

    printf("Bit format:                    %u-bit\n", h->bit_format == 1   ? 32
                                                      : h->bit_format == 2 ? 64
                                                                           : 0);

    printf("Endian:                        %s\n",
           h->endian == 1   ? "Little endian"
           : h->endian == 2 ? "Big endian"
                            : "Unknown");

    printf("Version:                       %u\n", h->version);
    printf("ABI:                           %u\n", h->ABI);
    printf("ABI specifics:                 %u\n", h->ABI_specifics);

    printf("\nFile info\n");
    printf("---------\n");
    uint16_t type = read_u16_le(h->Object_filetype);
    uint16_t machine = read_u16_le(h->TargetArchitecture);

    printf("Object file type:              0x%04x (%s)\n", type,
           elf_type(type));

    printf("Target architecture:           0x%04x (%s)\n", machine,
           elf_machine(machine));
    printf("ELF version:                   %u\n", read_u32_le(h->ElfVersion));
    printf("Flags:                         0x%08x\n", read_u32_le(h->flags));

    printf("\nOffsets / addresses\n");
    printf("-------------------\n");
    printf("Program entry point:           0x%016llx\n",
           (unsigned long long)read_u64_le(h->ProgrammEntryPoint));

    printf("Start of program header table: 0x%016llx\n",
           (unsigned long long)read_u64_le(h->StartOfHeaderTable));

    printf("Start of section header table: 0x%016llx\n",
           (unsigned long long)read_u64_le(h->StartSectionHeaderTable));

    printf("\nSizes / counts\n");
    printf("--------------\n");
    printf("Header size:                   %u bytes\n",
           read_u16_le(h->header_size));
    printf("Program header entry size:     %u bytes\n",
           read_u16_le(h->programm_header_table_entry_size));
    printf("Program header entries:        %u\n",
           read_u16_le(h->number_program_header_table_entries));
    printf("Section header entry size:     %u bytes\n",
           read_u16_le(h->header_section_size));
    printf("Section header entries:        %u\n",
           read_u16_le(h->number_of_entries_section_header_table));
    printf(
        "Section names index:           %u\n",
        read_u16_le(
            h->section_header_table_entry_which_contains_section_names_index));
}

void print_bytes(void *ptr, size_t size) {
    unsigned char *bytes = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        printf("0x%02X ", bytes[i]);
    }
    printf("\n");
}

int main() {
    FILE *file = fopen("elf", "rb");

    if (!file) {
        printf("ELF file not found. \n");
        return 1;
    }

    struct stat st;
    stat("elf", &st);
    size_t size = st.st_size;

    unsigned char bytes[size];
    fread(bytes, 1, size, file);

    ELF_HEADER header;

    Byte *start = (Byte *)&header;

    for (int i = 0; i < sizeof(header); i++) {
        *(start + i) = bytes[i];
    }

    print_elf_header(&header);

    return 0;
}
