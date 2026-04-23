#include "elf--later/elf.h"
#include "elf.h"
#include "asm.h"
#include "Logger.h"
#include "string.h"
#include "FileSystem.h"
#include "memory.h"
#include "arch_paging.h"
#include "scheduler.h"

#include <assert.h>
#include <stdint.h>

extern char _kernel_start;
extern char _kernel_end;

bool elf_check_support(Elf32_Ehdr* elf_header, const char* path){
    Sys_log("Checking ELF support for: %s\n", path);

    if (elf_header->e_ident[0] != ELFMAG0 ||
        elf_header->e_ident[1] != ELFMAG1 ||
        elf_header->e_ident[2] != ELFMAG2 || 
        elf_header->e_ident[3] != ELFMAG3) {
        Sys_log("invalid elf magic in %s\n", path);
        return false;
    }

    if(elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
        Sys_Error("Unsupported ELF File Class.\n");
        return false;
    }
    if(elf_header->e_ident[EI_DATA] != ELFDATA2LSB) {
        Sys_Error("Unsupported ELF File byte order.\n");
        return false;
    }
    if(elf_header->e_machine != EM_386) {
        Sys_Error("Unsupported ELF File target.\n");
        return false;
    }
    if(elf_header->e_ident[EI_VERSION] != EV_CURRENT) {
        Sys_Error("Unsupported ELF File version.\n");
        return false;
    }
    if(elf_header->e_type != ET_EXEC){
        Sys_Error("unsupported elf type %d in %s\n", elf_header->e_type, path);
        return false;
    }
    
    if (elf_header->e_phentsize != sizeof(Elf32_Phdr)) {
        Sys_Error("unexpected program header size in %s\n", path);
        return false;
    }
    if (elf_header->e_phnum == 0) {
        Sys_Error("program headers in %s\n", path);
        return false;
    }


    Sys_Success("ELF header is valid\n");

    return true;
}

pid_t LoadElf(const char* path) {
#if ELF_DEBUG
    Sys_log("Opening ELF file: %s\n", path);
#endif

    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        Sys_Error("Failed to open file '%s', error code: %d\n", path, res);
        return -1;
    }

    Elf32_Ehdr elf_header;
    UINT bytesRead;
    res = f_read(&file, &elf_header, sizeof(Elf32_Ehdr), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(Elf32_Ehdr)) {
        Sys_Error("Failed to read ELF header from '%s'\n", path);
        f_close(&file);
        return -1;
    }

    if (!elf_check_support(&elf_header, path)) {
        Sys_Error("ELF file '%s' is not supported\n", path);
        f_close(&file);
        return -1;
    }

#if ELF_DEBUG
    Sys_log("ELF entry point: 0x%x\n", elf_header.e_entry);
#endif

    f_lseek(&file, elf_header.e_phoff);
    Elf32_Phdr* program_headers = kmalloc(sizeof(Elf32_Phdr) * elf_header.e_phnum);
    
    if (!program_headers) {
        Sys_log("Failed to allocate memory for program headers\n");
        f_close(&file);
        return -1;
    }

    res = f_read(&file, program_headers, sizeof(Elf32_Phdr) * elf_header.e_phnum, &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(Elf32_Phdr) * elf_header.e_phnum) {
        Sys_log("Failed to read program headers\n");
        kfree(program_headers);
        f_close(&file);
        return -1;
    }

    PD_t app_page_dir;

    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf32_Phdr* phdr = &program_headers[i];
        if (phdr->p_type != PT_LOAD) continue;

        size_t num_pages = (phdr->p_memsz + PAGE_SIZE - 1) / PAGE_SIZE;
        uintptr_t segment_mem = page_alloc(num_pages, ((phdr->p_flags & PF_W) ? PAGE_FLAG_RW : 0) | PAGE_FLAG_USER) << 12;
        if (!segment_mem) {
            Sys_log("Failed to allocate memory for segment %d\n", i);
            kfree(program_headers);
            f_close(&file);
            return -1;
        }

#if ELF_DEBUG
        Sys_log("Segment %d -> VA: 0x%x, PA: 0x%x, Size: %u bytes \n", i, phdr->p_vaddr, segment_mem, phdr->p_memsz, i);
#endif

        pd_map_page(&app_page_dir, phdr->p_vaddr, segment_mem >> 12, 1, (phdr->p_flags & PF_W) ? 1 : 0, 1);

        f_lseek(&file, phdr->p_offset);
        res = f_read(&file, (void*)segment_mem, phdr->p_filesz, &bytesRead);
        if (res != FR_OK || bytesRead != phdr->p_filesz) {
            Sys_log("Failed to read segment data for segment %d\n", i);
            kfree(program_headers);
            f_close(&file);
            return -1;
        }

        if (phdr->p_memsz > phdr->p_filesz) {
            memset((void*)(segment_mem + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    kfree(program_headers);

    pid_t pid = us_task_start((void*)elf_header.e_entry, (char*)path, app_page_dir);

    f_close(&file);
#if ELF_DEBUG
    Sys_log("ELF file '%s' loaded successfully. Entry: 0x%x\n", path, elf_header.e_entry);
#endif
    return pid;
}





