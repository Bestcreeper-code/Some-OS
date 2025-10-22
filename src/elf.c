#include "headers/elf.h"
#include "headers/Logger.h"
#include "headers/string.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"
#include "headers/paging.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


extern char _kernel_start;
extern char _kernel_end;

bool elf_check_support(Elf32_Ehdr* elf_header, const char* path){
    if (elf_header->e_ident[0] != ELFMAG0 ||
         elf_header->e_ident[1] != ELFMAG1 ||
        elf_header->e_ident[2] != ELFMAG2 || 
        elf_header->e_ident[3] != ELFMAG3) {
        Sys_log("invalid elf magic in %s\n", path);
        return false;
    }

    if(elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
		Sys_log("Unsupported ELF File Class.\n");
		return false;
	}
	if(elf_header->e_ident[EI_DATA] != ELFDATA2LSB) {
		Sys_log("Unsupported ELF File byte order.\n");
		return false;
	}
	if(elf_header->e_machine != EM_386) {
		Sys_log("Unsupported ELF File target.\n");
		return false;
	}
	if(elf_header->e_ident[EI_VERSION] != EV_CURRENT) {
		Sys_log("Unsupported ELF File version.\n");
		return false;
	}

    if(elf_header->e_type != ET_EXEC){
        Sys_log("unsupported elf type %d in %s\n", elf_header->e_type, path);
        return false;
    }
    
    if (elf_header->e_phentsize != sizeof(Elf32_Phdr)) {
        Sys_log("unexpected program header size in %s\n", path);
        return false;
    }

    return true;
}

LoadedElf* LoadElf(const char* path) {
    Sys_log("Opening ELF file: %s\n", path);

    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        Sys_log("Failed to open file '%s', error code: %d\n", path, res);
        return NULL;
    }

    Elf32_Ehdr elf_header;
    UINT bytesRead;
    res = f_read(&file, &elf_header, sizeof(Elf32_Ehdr), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(Elf32_Ehdr)) {
        Sys_log("Failed to read ELF header from '%s'\n", path);
        f_close(&file);
        return NULL;
    }

    if (!elf_check_support(&elf_header, path)) {
        Sys_log("ELF file '%s' is not supported\n", path);
        f_close(&file);
        return NULL;
    }

    Sys_log("ELF header read successfully. Entry point: 0x%08X\n", elf_header.e_entry);
    f_lseek(&file, elf_header.e_phoff);

    Elf32_Phdr* program_headers = malloc(sizeof(Elf32_Phdr) * elf_header.e_phnum);
    if (!program_headers) {
        Sys_log("Failed to allocate memory for program headers\n");
        f_close(&file);
        return NULL;
    }

    res = f_read(&file, program_headers, sizeof(Elf32_Phdr) * elf_header.e_phnum, &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(Elf32_Phdr) * elf_header.e_phnum) {
        Sys_log("Failed to read program headers\n");
        free(program_headers);
        f_close(&file);
        return NULL;
    }

    
    PD_t app_page_dir;

    // Allocate page_groups for kernel + segments + stack
    Page_Group* page_groups = malloc(sizeof(Page_Group) * (elf_header.e_phnum + 2));
    if (!page_groups) {
        Sys_log("Failed to allocate page_groups\n");
        free(program_headers);
        f_close(&file);
        return NULL;
    }
    memset(page_groups, 0, sizeof(Page_Group) * (elf_header.e_phnum + 2));

    // Identity map kernel region as the first page group
    uintptr_t kernel_start = (uintptr_t)&_kernel_start;
    uintptr_t kernel_end = (uintptr_t)&_kernel_end;

    uintptr_t id_map_start = kernel_start & ~(_PAGE_SIZE - 1);
    uintptr_t id_map_end = kernel_end + (_PAGE_SIZE - 1) & ~(_PAGE_SIZE - 1);
    size_t kernel_pages = (id_map_end - id_map_start) / _PAGE_SIZE;

    Sys_log("mapping kernel: 0x%x - 0x%x (%u pages)\n", id_map_start, id_map_end, kernel_pages);

    page_groups[0] = (Page_Group){
        .size = kernel_pages,
        .addr = id_map_start,
        .pte_bits = {
            .present = 1,
            .rw = 1,
            .user = 0, // kernel space
            .addr = id_map_start >> 12
        }
    };


    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf32_Phdr* phdr = &program_headers[i];

        if (phdr->p_type != PT_LOAD) continue;

        size_t num_pages = (phdr->p_memsz + _PAGE_SIZE - 1) / _PAGE_SIZE;
        uintptr_t phdr_vaddr = phdr->p_vaddr;

        page_addr_t segment_mem = page_alloc(num_pages, (phdr->p_flags & PF_W) ? 1 : 0, 1);
        if (!segment_mem) {
            Sys_log("Failed to allocate memory for segment %d\n", i);
            for (int j = 0; j < i; j++) {
                if (page_groups[j].size)
                    page_free(page_groups[j].pte_bits.addr << 12, page_groups[j].size);
            }
            free(page_groups);
            free(program_headers);
            f_close(&file);
            return NULL;
        }

        Sys_log("Segment %d -> VA: 0x%x, PA: 0x%x, Size: %u bytes\n",
            i, phdr_vaddr, segment_mem, phdr->p_memsz);

        page_groups[i+1] = (Page_Group){
            .size = num_pages,
            .addr = phdr_vaddr,
            .pte_bits = {
                .present = 1,
                .rw = (phdr->p_flags & PF_W) ? 1 : 0,
                .user = 1,
                .addr = segment_mem >> 12
            }
        };

        f_lseek(&file, phdr->p_offset);
        res = f_read(&file, (void*)segment_mem, phdr->p_filesz, &bytesRead);
        if (res != FR_OK || bytesRead != phdr->p_filesz) {
            Sys_log("Failed to read segment data for segment %d\n", i);
            free(page_groups);
            free(program_headers);
            f_close(&file);
            return NULL;
        }

        if (phdr->p_memsz > phdr->p_filesz) {
            uintptr_t bss_start = segment_mem + phdr->p_filesz;
            size_t bss_size = phdr->p_memsz - phdr->p_filesz;
            memset((void*)bss_start, 0, bss_size);
        }
    }

    // Allocate stack
    page_addr_t app_stack = page_alloc(DEFAULT_STACK_PAGE_AMOUNT, 1, 1);
    if (!app_stack) {
        Sys_log("Failed to allocate stack\n");
        pd_free(&app_page_dir);
        free(page_groups);
        free(program_headers);
        f_close(&file);
        return NULL;
    }

    Sys_log("Stack allocated at PA: 0x%08X\n", app_stack);

    page_groups[elf_header.e_phnum] = (Page_Group){
        .size = DEFAULT_STACK_PAGE_AMOUNT,
        .addr = DEFAULT_STACK_BOTTOM_VADDR,
        .pte_bits = {
            .present = 1,
            .rw = 1,
            .user = 1,
            .addr = app_stack >> 12
        }
    };

    new_page_dir(page_groups, elf_header.e_phnum + 2, &app_page_dir); // includes stack, segs and kernel space(mapped as u/s 0)
    free(program_headers);
    free(page_groups);

    LoadedElf* loaded_elf = malloc(sizeof(LoadedElf));
    if (!loaded_elf) {
        Sys_log("Failed to allocate LoadedElf struct\n");
        pd_free(&app_page_dir);
        f_close(&file);
        return NULL;
    }

    loaded_elf->entry_point = elf_header.e_entry;
    loaded_elf->page_dir = app_page_dir;
    loaded_elf->stack_bottom = DEFAULT_STACK_BOTTOM_VADDR;
    loaded_elf->stack_top = DEFAULT_STACK_TOP_VADDR;
    loaded_elf->filename = strdup(path);

    if (!loaded_elf->filename) {
        Sys_log("strdup failed for path\n");
        pd_free(&app_page_dir);
        free(loaded_elf);
        f_close(&file);
        return NULL;
    }

    f_close(&file);
    Sys_log("ELF file '%s' loaded successfully. Entry: 0x%x\n", path, loaded_elf->entry_point);
    return loaded_elf;
}

