#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

#include "paging.h"

#define EI_NIDENT 16
#define SHN_UNDEF 0

#define DEFAULT_STACK_PAGE_AMOUNT 32
#define DEFAULT_STACK_TOP_VADDR        0xC0000000 
#define DEFAULT_STACK_BOTTOM_VADDR    DEFAULT_STACK_TOP_VADDR - (DEFAULT_STACK_PAGE_AMOUNT * _PAGE_SIZE) 




typedef struct {
    uint8_t       e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint32_t      e_entry;
    uint32_t      e_phoff;
    uint32_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf32_Ehdr;


enum Elf_Ident {
    EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};

# define ELFMAG0	0x7F // e_ident[EI_MAG0]
# define ELFMAG1	'E'  // e_ident[EI_MAG1]
# define ELFMAG2	'L'  // e_ident[EI_MAG2]
# define ELFMAG3	'F'  // e_ident[EI_MAG3]

# define ELFDATA2LSB 1   // Little Endian
# define ELFCLASS32	 1   // 32-bit Architecture

# define EM_386		 3
# define EV_CURRENT	 1  // ELF Current Version

typedef struct {
    uint32_t p_type;    // Segment type
    uint32_t p_offset;  // File offset
    uint32_t p_vaddr;   // Virtual address
    uint32_t p_paddr;   // Physical address
    uint32_t p_filesz;  // Size in file
    uint32_t p_memsz;   // Size in memory
    uint32_t p_flags;   // Flags (R/W/X)
    uint32_t p_align;   // Alignment
} Elf32_Phdr;

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


typedef enum {
    PF_X = 1 << 0,  
    PF_W = 1 << 1,  
    PF_R = 1 << 2   
} Elf32_SegmentFlags;


typedef enum {
    ET_NONE = 0, // No file type
    ET_REL  = 1, // Relocatable
    ET_EXEC = 2, // Executable
    ET_DYN  = 3, // Shared object
    ET_CORE = 4  // Core file
} Elf32_FileType;


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


typedef struct {
    uint32_t st_name;   // Index into string table
    uint32_t st_value;  // Value (address or offset)
    uint32_t st_size;   // Size in bytes
    uint8_t  st_info;   // Type and binding
    uint8_t  st_other;  // Visibility
    uint16_t st_shndx;  // Section index
} Elf32_Sym;


typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;


#define ELF32_R_SYM(info)  ((info) >> 8)
#define ELF32_R_TYPE(info) ((uint8_t)(info))


typedef enum {
    R_386_NONE     = 0,
    R_386_32       = 1, // S + A
    R_386_PC32     = 2, // S + A - P
    R_386_RELATIVE = 8  // B + A
} Elf32_RelocationType;


typedef struct {
    char* filename;

    PD_t page_dir;

    uintptr_t entry_point;

    uintptr_t stack_top;
    uintptr_t stack_bottom;
} LoadedElf;


LoadedElf* LoadElf(const char* path);


#endif // ELF_H
