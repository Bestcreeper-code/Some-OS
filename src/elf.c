#include "headers/elf.h"
#include "headers/Logger.h"
#include "headers/string.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"
#include "headers/paging.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint32_t align_up(uint32_t value, uint32_t align) {
    if (!align) return value;
    return (value + align - 1) & ~(align - 1);
}

int Runelf(const char* path, int argc, char** argv) {
    Sys_log("Runelf: loading %s\n", path);
    LoadedElf* elf_file = LoadElf(path);
    if (!elf_file) {
        Sys_log("Runelf: LoadElf returned NULL\n");
        return -1;
    }

    uintptr_t addr = ELF_GetSymbol(elf_file, "_start");
    if (!addr) {
        Sys_log("Runelf: _start not found\n");
        return -2;
    }

    Sys_log("Runelf: elf mem=%p size=%u entry=%p\n",
            elf_file->mem, (unsigned)elf_file->size, (void*)addr);

    uintptr_t mem = (uintptr_t)elf_file->mem;
    if (addr < mem || addr >= mem + elf_file->size) {
        Sys_log("Runelf ERROR: entry %p outside loaded region [%p - %p]\n",
                (void*)addr, (void*)mem, (void*)(mem + elf_file->size));
        return -3;
    }

    uint8_t *b = (uint8_t*)addr;
#if ELF_DEBUG_MODE
    Sys_log("Runelf: first bytes at entry: %x %x %x %x %x %x %x %x\n",
            b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
#endif

    int (*entry)(int, char**) = (int (*)(int, char**))addr;
    Sys_log("Runelf: jumping to _start\n");
    return entry(argc, argv);
}

char* create_app_vmem(uint32_t page_amount){
    uint32_t _pages =page_alloc(page_amount,1,1); 
    Page_Group pages = {
        .size = page_amount,
        .addr = 0x08048000,
        .pte_bits = {
            .present = 1,
            .rw = 1,
            .user = 1,
            .pwt = 0,
            .pcd = 0,
            .accessed = 0,
            .dirty = 0,
            .pat = 0,
            .global = 0,
            .os_allocated = 0,
            .os_unallocatable = 0,
            .os_unused = 0,
        }
    };
    char* page_dir = new_page_dir()
}

LoadedElf* LoadElf(const char* path) {
    Sys_log("LoadElf: opening %s\n", path);

    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) return NULL;

    Elf32_Ehdr ehdr;
    UINT bytesRead;
    res = f_read(&file, &ehdr, sizeof(ehdr), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(ehdr)) { f_close(&file); return NULL; }

    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        f_close(&file);
        return NULL;
    }

    if (ehdr.e_type != ET_REL) {
        Sys_log("LoadElf ERROR: not a REL ELF (e_type=%d)\n", ehdr.e_type);
        f_close(&file);
        return NULL;
    }

    Elf32_Shdr* sections = malloc(sizeof(Elf32_Shdr) * ehdr.e_shnum);
    uint32_t* section_offsets = malloc(sizeof(uint32_t) * ehdr.e_shnum);
    if (!sections || !section_offsets) {
        free(sections); free(section_offsets); f_close(&file); return NULL;
    }

    for (int i = 0; i < ehdr.e_shnum; i++) {
        f_lseek(&file, ehdr.e_shoff + i * sizeof(Elf32_Shdr));
        f_read(&file, &sections[i], sizeof(Elf32_Shdr), &bytesRead);
        if (bytesRead != sizeof(Elf32_Shdr)) {
            free(sections); free(section_offsets); f_close(&file); return NULL;
        }
    }

    uint32_t max_size = 0;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        uint32_t align = sections[i].sh_addralign ? sections[i].sh_addralign : 1;
        max_size = align_up(max_size, align);
        section_offsets[i] = max_size; // always store offset, even if sh_size == 0
        max_size += sections[i].sh_size;
    }

    uint32_t pages_needed = (max_size + _PAGE_SIZE - 1) / _PAGE_SIZE;
    uint32_t base_pa = page_alloc(pages_needed, 1, 0);
    if (!base_pa) { free(sections); free(section_offsets); f_close(&file); return NULL; }
    uint8_t* base = (uint8_t*)(uintptr_t)base_pa;
    memset(base, 0, pages_needed * _PAGE_SIZE);

    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (!sections[i].sh_size) continue;
        if (sections[i].sh_type == SHT_NOBITS) { memset(base + section_offsets[i], 0, sections[i].sh_size); continue; }
        f_lseek(&file, sections[i].sh_offset);
        f_read(&file, base + section_offsets[i], sections[i].sh_size, &bytesRead);
        if (bytesRead != sections[i].sh_size) { page_free(base_pa, pages_needed); free(sections); free(section_offsets); f_close(&file); return NULL; }
    }

    // Relocations
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (sections[i].sh_type != SHT_REL) continue;

        uint32_t rel_count = sections[i].sh_size / sizeof(Elf32_Rel);
        f_lseek(&file, sections[i].sh_offset);

        for (uint32_t j = 0; j < rel_count; j++) {
            Elf32_Rel rel;
            f_read(&file, &rel, sizeof(rel), &bytesRead);
            if (bytesRead != sizeof(rel)) { page_free(base_pa, pages_needed); free(sections); free(section_offsets); f_close(&file); return NULL; }

            uint32_t target_index = sections[i].sh_info;
            if (target_index >= ehdr.e_shnum) continue;

            uint32_t* patch = (uint32_t*)(base + section_offsets[target_index] + rel.r_offset);
            uint32_t sym_index = ELF32_R_SYM(rel.r_info);
            uint32_t symtab_index = sections[i].sh_link;
            if (symtab_index >= ehdr.e_shnum) { page_free(base_pa, pages_needed); free(sections); free(section_offsets); f_close(&file); return NULL; }

            Elf32_Sym* symtab = (Elf32_Sym*)(base + section_offsets[symtab_index]);
            Elf32_Sym sym = symtab[sym_index];

            uintptr_t sym_addr = 0;
            if (sym.st_shndx != SHN_UNDEF && sym.st_shndx < ehdr.e_shnum)
                sym_addr = (uintptr_t)(base + section_offsets[sym.st_shndx] + sym.st_value);

            uint32_t orig = *patch;
            switch (ELF32_R_TYPE(rel.r_info)) {
                case R_386_32: *patch = orig + (uint32_t)sym_addr; break;
                case R_386_PC32: *patch = orig + (uint32_t)sym_addr - (uint32_t)(uintptr_t)patch; break;
                case R_386_RELATIVE: *patch = orig + (uint32_t)(uintptr_t)base; break;
                default: page_free(base_pa, pages_needed); free(sections); free(section_offsets); f_close(&file); return NULL;
            }
        }
    }

    LoadedElf* elf = malloc(sizeof(LoadedElf));
    if (!elf) { page_free(base_pa, pages_needed); free(sections); free(section_offsets); f_close(&file);Sys_log("bruhhhh"); return NULL; }
    elf->mem = base;
    elf->sections = sections;
    elf->section_offsets = section_offsets;
    elf->shnum = ehdr.e_shnum;
    elf->size = max_size;
    elf->entry = 0;

    f_close(&file);
    return elf;
}

uintptr_t ELF_GetSymbol(LoadedElf* elf, const char* name) {
    if (!elf || !name) return 0;

    uint32_t* offs = elf->section_offsets;

    for (int i = 0; i < elf->shnum; i++) {
        Elf32_Shdr sh = elf->sections[i];
        if (sh.sh_type != SHT_SYMTAB) continue;

        Elf32_Sym* symtab = (Elf32_Sym*)((uint8_t*)elf->mem + offs[i]);
        int count = sh.sh_size / sizeof(Elf32_Sym);
        int strtab_index = sh.sh_link;
        if (strtab_index < 0 || strtab_index >= elf->shnum) continue;

        char* strtab_mem = (char*)((uint8_t*)elf->mem + offs[strtab_index]);

        for (int j = 0; j < count; j++) {
            Elf32_Sym* s = &symtab[j];
            if (s->st_name == 0) continue;

            const char* sym_name = strtab_mem + s->st_name;
            if (!sym_name) continue;

#if ELF_DEBUG_MODE
            Sys_log("%s\n", sym_name);
#endif
            if (strcmp(sym_name, name) == 0) {
                if (s->st_shndx == SHN_UNDEF || s->st_shndx >= elf->shnum) return 0;

                uint32_t sec_offset = offs[s->st_shndx];

#if ELF_DEBUG_MODE
                Sys_log("Symbol %s found at %p\n", name, (void*)(elf->mem + sec_offset + s->st_value));
#endif
                return (uintptr_t)((uint8_t*)elf->mem + sec_offset + s->st_value);
            }
        }
    }

    return 0;
}
