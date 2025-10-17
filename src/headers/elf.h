#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

#define EI_NIDENT 16
#define SHN_UNDEF 0



// === ELF Header ===
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint32_t      e_entry;      // Entry point
    uint32_t      e_phoff;      // Program header table offset
    uint32_t      e_shoff;      // Section header table offset
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;      // Number of program headers
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf32_Ehdr;

// === Program Header ===
typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

// Program header types
typedef enum {
    PT_NULL    = 0,
    PT_LOAD    = 1,
    PT_DYNAMIC = 2,
    PT_INTERP  = 3,
    PT_NOTE    = 4,
    PT_SHLIB   = 5,
    PT_PHDR    = 6,
    PT_TLS     = 7
} Elf32_SegmentType;

// Program header flags
typedef enum {
    PF_X = 1 << 0,  // Execute
    PF_W = 1 << 1,  // Write
    PF_R = 1 << 2   // Read
} Elf32_SegmentFlags;

// File types
typedef enum {
    ET_NONE = 0, // No file type
    ET_REL  = 1, // Relocatable
    ET_EXEC = 2, // Executable
    ET_DYN  = 3, // Shared object
    ET_CORE = 4  // Core file
} Elf32_FileType;

// === Section Header ===
typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr;

// Section types
typedef enum {
    SHT_NULL     = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB   = 2,
    SHT_STRTAB   = 3,
    SHT_RELA     = 4,
    SHT_HASH     = 5,
    SHT_DYNAMIC  = 6,
    SHT_NOTE     = 7,
    SHT_NOBITS   = 8,
    SHT_REL      = 9
} Elf32_SectionType;

// === Symbol Table Entry ===
typedef struct {
    uint32_t st_name;   // Index into string table
    uint32_t st_value;  // Value (address or offset)
    uint32_t st_size;   // Size in bytes
    uint8_t  st_info;   // Type and binding
    uint8_t  st_other;  // Visibility
    uint16_t st_shndx;  // Section index
} Elf32_Sym;

// === Relocation Entry (no addend) ===
typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;

// Relocation macros
#define ELF32_R_SYM(info)  ((info) >> 8)
#define ELF32_R_TYPE(info) ((uint8_t)(info))

// Relocation types for x86
typedef enum {
    R_386_NONE     = 0,
    R_386_32       = 1, // S + A
    R_386_PC32     = 2, // S + A - P
    R_386_RELATIVE = 8  // B + A
} Elf32_RelocationType;

// === Loaded ELF ===
typedef struct {
    uint8_t* mem;
    size_t size;
    uint32_t entry;
    Elf32_Shdr* sections;
    int shnum;
    uint32_t* section_offsets; // numeric offsets into mem
} LoadedElf;




/*============================================================
 * Functions
 *===========================================================*/

int Runelf(const char* path, int argc, char** argv);

LoadedElf* LoadElf(const char* path);
uint32_t ELF_GetSymbol(LoadedElf* elf, const char* name);


#endif // ELF_H
