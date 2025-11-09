#include "headers/elf.h"
#include "headers/Logger.h"
#include "headers/string.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"
#include "headers/paging.h"
#include "headers/scheduler.h"

#include <stdint.h>

extern char _kernel_start;
extern char _kernel_end;

bool elf_check_support(Elf32_Ehdr* elf_header, const char* path){
#if ELF_DEBUG_MODE
    Sys_log("Checking ELF support for: %s\n", path);
#endif

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
    if (elf_header->e_phnum == 0) {
        Sys_log("program headers in %s\n", path);
        return false;
    }

#if ELF_DEBUG_MODE
    Sys_log("ELF header is valid\n");
#endif
    return true;
}

LoadedElf* LoadElf(const char* path) {
#if ELF_DEBUG_MODE
    Sys_log("Opening ELF file: %s\n", path);
#endif

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

#if ELF_DEBUG_MODE
    Sys_log("ELF entry point: 0x%x\n", elf_header.e_entry);
#endif

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
    Page_Group* page_groups = malloc(sizeof(Page_Group) * (elf_header.e_phnum + 2));
    if (!page_groups) {
        Sys_log("Failed to allocate page_groups\n");
        free(program_headers);
        f_close(&file);
        return NULL;
    }
    memset(page_groups, 0, sizeof(Page_Group) * (elf_header.e_phnum + 2));

    uintptr_t kernel_start = (uintptr_t)&_kernel_start;
    uintptr_t kernel_end   = (uintptr_t)&_kernel_end;
    uintptr_t id_map_start = kernel_start & ~(_PAGE_SIZE - 1);
    uintptr_t id_map_end   = (kernel_end + (_PAGE_SIZE - 1)) & ~(_PAGE_SIZE - 1);
    size_t kernel_pages    = (id_map_end - id_map_start) / _PAGE_SIZE;

#if ELF_DEBUG_MODE
    Sys_log("Mapping kernel: 0x%x - 0x%x (%u pages)\n", id_map_start, id_map_end, kernel_pages);
#endif

    page_groups[0] = (Page_Group){
        .size = kernel_pages,
        .addr = id_map_start,
        .pte_bits = {.present = 1, .rw = 1, .user = 0, .addr = id_map_start >> 12}
    };

    //pt_load segs 
    int seg_index = 0;
    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf32_Phdr* phdr = &program_headers[i];
        if (phdr->p_type != PT_LOAD) continue;

        size_t num_pages = (phdr->p_memsz + _PAGE_SIZE - 1) / _PAGE_SIZE;
        uintptr_t phdr_vaddr = phdr->p_vaddr;
        page_addr_t segment_mem = page_alloc(num_pages, (phdr->p_flags & PF_W) ? 1 : 0, 1);
        if (!segment_mem) {
            Sys_log("Failed to allocate memory for segment %d\n", i);
            //free segs page
            for (int j = 0; j < seg_index; j++) {
                if (page_groups[j+1].size)
                    page_free(page_groups[j+1].pte_bits.addr << 12, page_groups[j+1].size);
            }
            free(page_groups);
            free(program_headers);
            f_close(&file);
            return NULL;
        }

#if ELF_DEBUG_MODE
        Sys_log("Segment %d -> VA: 0x%x, PA: 0x%x, Size: %u bytes\n", seg_index, phdr_vaddr, segment_mem, phdr->p_memsz);
#endif

        page_groups[seg_index+1] = (Page_Group){
            .size = num_pages,
            .addr = phdr_vaddr,
            .pte_bits = {.present = 1, .rw = (phdr->p_flags & PF_W) ? 1 : 0, .user = 1, .addr = segment_mem >> 12}
        };
        seg_index++;

        f_lseek(&file, phdr->p_offset);
        res = f_read(&file, (void*)segment_mem, phdr->p_filesz, &bytesRead);
        if (res != FR_OK || bytesRead != phdr->p_filesz) {
            Sys_log("Failed to read segment data for segment %d\n", i);
            //free segs pages
            for (int j = 0; j < seg_index; j++) {
                if (page_groups[j+1].size)
                    page_free(page_groups[j+1].pte_bits.addr << 12, page_groups[j+1].size);
            }
            free(page_groups);
            free(program_headers);
            f_close(&file);
            return NULL;
        }

        if (phdr->p_memsz > phdr->p_filesz) {
            memset((void*)(segment_mem + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    //stack

    page_addr_t stack_pa = page_alloc(DEFAULT_STACK_PAGE_AMOUNT, 1, 1);
    if (!stack_pa) {
        Sys_log("Failed to allocate stack\n");
        
        for (int j = 0; j < seg_index; j++) {
            if (page_groups[j+1].size)
                page_free(page_groups[j+1].pte_bits.addr << 12, page_groups[j+1].size);
        }
        free(page_groups);
        free(program_headers);
        f_close(&file);
        return NULL;
    }

#if ELF_DEBUG_MODE
    Sys_log("Stack physical allocated at PA: 0x%x (pages: %u)\n", stack_pa, (unsigned)DEFAULT_STACK_PAGE_AMOUNT);
#endif

    // map stack
    uintptr_t stack_v_bottom = DEFAULT_STACK_TOP_VADDR - (DEFAULT_STACK_PAGE_AMOUNT * _PAGE_SIZE);
    page_groups[seg_index+1] = (Page_Group){
        .size = DEFAULT_STACK_PAGE_AMOUNT,
        .addr = stack_v_bottom,
        .pte_bits = {.present = 1, .rw = 1, .user = 1, .addr = stack_pa >> 12}
    };

    //make pd
    new_page_dir(page_groups, seg_index + 2, &app_page_dir);

    free(program_headers);
    free(page_groups);

    LoadedElf* loaded_elf = malloc(sizeof(LoadedElf));
    if (!loaded_elf) {
        Sys_log("Failed to allocate LoadedElf struct\n");
        pd_free(&app_page_dir);
        f_close(&file);
        return NULL;
    }

    loaded_elf->entry_point  = elf_header.e_entry;
    loaded_elf->page_dir     = app_page_dir;
    
    loaded_elf->ph_stack_bottom = stack_pa;
    loaded_elf->ph_stack_top    = stack_pa + (DEFAULT_STACK_PAGE_AMOUNT * _PAGE_SIZE);
    loaded_elf->v_esp          = DEFAULT_STACK_TOP_VADDR;
    loaded_elf->filename     = (path);

    f_close(&file);
#if ELF_DEBUG_MODE
    Sys_log("ELF file '%s' loaded successfully. Entry: 0x%x\n", path, loaded_elf->entry_point);
    Sys_log("LoadedElf: phys_stack_bottom=0x%x phys_stack_top=0x%x virt_esp=0x%x\n",
            loaded_elf->ph_stack_bottom, loaded_elf->ph_stack_top, loaded_elf->v_esp);
#endif
    return loaded_elf;
}




ProcessInfo exec_ELF(char* path){
#if ELF_DEBUG_MODE
    Sys_log("Executing ELF: %s\n", path);
#endif

    LoadedElf* elf = LoadElf(path);
    if(!elf) {
#if ELF_DEBUG_MODE
        Sys_log("Failed to load ELF: %s\n", path);
#endif
        return (ProcessInfo){0}; // pid 0 = error
    }

    _setup_user_stack_sched_frame((void*)elf->ph_stack_top, &elf->v_esp, (uint32_t)elf->entry_point);
    uint32_t ebp = DEFAULT_STACK_TOP_VADDR;
    pid_t pid = new_pcb(&elf->page_dir, elf->filename, &elf->v_esp, &ebp);
    if(pid <0){
#if ELF_DEBUG_MODE
        Sys_log("Failed to create PCB for ELF: %s (%d)\n", elf->filename, pid);
#endif
        pd_free(&elf->page_dir);
        free(elf->filename);
        free(elf);
        return (ProcessInfo){0};
    }
    
    ProcessInfo p_info = {0};
    p_info.pid = pid;
    p_info.name = elf->filename;

#if ELF_DEBUG_MODE
    Sys_log("ELF '%s' launched successfully. PID: %d, Entry: 0x%x\n", elf->filename, pid, elf->entry_point);
#endif

    free(elf);

    return p_info;
}
