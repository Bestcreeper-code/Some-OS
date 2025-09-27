#include "headers/elf.h"
#include "headers/Logger.h"
#include "headers/string.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"




LoadedElf* LoadElf(const char* path) {
    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
#if DEBUG_MODE
        Sys_log("LoadElf: Failed to open file %s\n", path);
#endif
        return NULL;
    }
#if DEBUG_MODE
    Sys_log("LoadElf: Opened file %s\n", path);
#endif

    Elf32_Ehdr ehdr;
    UINT bytesRead;
    res = f_read(&file, &ehdr, sizeof(Elf32_Ehdr), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(Elf32_Ehdr)) {
#if DEBUG_MODE
        Sys_log("LoadElf: Failed to read ELF header\n");
#endif
        return NULL;
    }

    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
#if DEBUG_MODE
        Sys_log("LoadElf: Not a valid ELF file\n");
#endif
        return NULL;
    }

    if (ehdr.e_type != ET_REL) {
#if DEBUG_MODE
        Sys_log("LoadElf: Not a relocatable ELF\n");
#endif
        return NULL;
    }

#if DEBUG_MODE
    Sys_log("LoadElf: Reading %u section headers\n", ehdr.e_shnum);
#endif

    Elf32_Shdr* sections = malloc(sizeof(Elf32_Shdr) * ehdr.e_shnum);
    uint32_t* section_offsets = malloc(sizeof(uint32_t) * ehdr.e_shnum);
    if (!sections || !section_offsets) {
#if DEBUG_MODE
        Sys_log("LoadElf: Failed to allocate memory for section headers\n");
#endif
        return NULL;
    }

    for (int i = 0; i < ehdr.e_shnum; i++) {
        f_lseek(&file, ehdr.e_shoff + i * sizeof(Elf32_Shdr));
        f_read(&file, &sections[i], sizeof(Elf32_Shdr), &bytesRead);
    }

    uint32_t max_size = 0;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        section_offsets[i] = max_size;
        if (sections[i].sh_type != SHT_NOBITS && sections[i].sh_size > 0)
            max_size += sections[i].sh_size;
    }

#if DEBUG_MODE
    Sys_log("LoadElf: Total memory needed for sections = %u bytes\n", max_size);
#endif

    uint8_t* base = malloc(max_size);
    if (!base) {
#if DEBUG_MODE
        Sys_log("LoadElf: Failed to allocate memory for ELF sections\n");
#endif
        free(sections);
        free(section_offsets);
        return NULL;
    }
    memset(base, 0, max_size);

#if DEBUG_MODE
    Sys_log("LoadElf: Loading section data\n");
#endif

    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (sections[i].sh_type == SHT_NOBITS || sections[i].sh_size == 0) continue;
        f_lseek(&file, sections[i].sh_offset);
        f_read(&file, base + section_offsets[i], sections[i].sh_size, &bytesRead);
    }

#if DEBUG_MODE
    Sys_log("LoadElf: Applying relocations\n");
#endif

    for (int i = 0; i < ehdr.e_shnum; i++) {
        Elf32_Shdr shdr = sections[i];
        if (shdr.sh_type != SHT_REL) continue;

        Elf32_Shdr target_sec = sections[shdr.sh_info];
        Elf32_Shdr symtab_sec = sections[shdr.sh_link];
        int rel_count = shdr.sh_size / sizeof(Elf32_Rel);

        f_lseek(&file, shdr.sh_offset);
        for (int j = 0; j < rel_count; j++) {
            Elf32_Rel rel;
            f_read(&file, &rel, sizeof(Elf32_Rel), &bytesRead);

            uint32_t* patch = (uint32_t*)(base + section_offsets[shdr.sh_info] + rel.r_offset);

            Elf32_Sym* symtab = (Elf32_Sym*)(base + section_offsets[shdr.sh_link]);
            Elf32_Sym sym = symtab[ELF32_R_SYM(rel.r_info)];
            uint32_t sym_addr = (sym.st_shndx < ehdr.e_shnum && sym.st_shndx != SHN_UNDEF) ?
                                (uint32_t)(base + section_offsets[sym.st_shndx] + sym.st_value) : 0;

            switch (ELF32_R_TYPE(rel.r_info)) {
                case R_386_32:   *patch += sym_addr; break;
                case R_386_PC32: *patch += sym_addr - (uint32_t)patch; break;
                case R_386_RELATIVE: *patch += (uint32_t)base; break;
            }
#if DEBUG_MODE
            Sys_log("LoadElf: Relocation %d applied at %p\n", j, patch);
#endif
        }
    }

    LoadedElf* elf = malloc(sizeof(LoadedElf));
    elf->mem = base;
    elf->sections = sections;
    elf->section_offsets = (uint8_t**)section_offsets;
    elf->shnum = ehdr.e_shnum;
    elf->size = max_size;
    elf->entry = (uint32_t)base;

    f_close(&file);
#if DEBUG_MODE
    Sys_log("LoadElf: ELF %s loaded successfully, entry=0x%x\n", path, elf->entry);
#endif

    return elf;
}


uint32_t GetSymbol(LoadedElf* elf, const char* name) {
    for (int i = 0; i < elf->shnum; i++) {
        Elf32_Shdr sh = elf->sections[i];
        if (sh.sh_type != SHT_SYMTAB) continue;

        Elf32_Sym* symtab = (Elf32_Sym*)(elf->mem + (uint32_t)elf->section_offsets[i]);
        Elf32_Shdr strtab = elf->sections[sh.sh_link];
        char* strtab_mem = (char*)(elf->mem + (uint32_t)elf->section_offsets[sh.sh_link]);
        int count = sh.sh_size / sizeof(Elf32_Sym);

        for (int j = 0; j < count; j++) {
            Sys_log("Symbol: %s\n", strtab_mem + symtab[j].st_name);
            if (strcmp(strtab_mem + symtab[j].st_name, name) == 0) {
                if (symtab[j].st_shndx == SHN_UNDEF) return 0;
                return (uint32_t)(elf->mem + (uint32_t) elf->section_offsets[symtab[j].st_shndx] + symtab[j].st_value);
            }
        }
    }
    return 0;
}

int JumpToSymbol(LoadedElf* elf, const char* name) {
    uint32_t addr = GetSymbol(elf, name);
    if (addr == 0) return -1;

    void (*func)() = (void (*)())addr;
    func();
    return 0;
}