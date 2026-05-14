#include "elf--later/elf.h"
#include "elf.h"
#include "asm.h"
#include "Logger.h"
#include "fs.h"
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


pid_t load_elf_from_vfs(const char* vfs_path) {
    struct dentry* d = kpath_lookup(root_dentry->inode, vfs_path);
    if (!d || !d->inode || !d->inode->i_fop) {
        Sys_Error("ELF: path not found: %s\n", vfs_path);
        return -1;
    }

    struct file f = {0};
    if (d->inode->i_fop->open(d->inode, &f) < 0) return -1;

    // Read ELF header
    Elf32_Ehdr ehdr;
    loff_t off = 0;
    if (d->inode->i_fop->read(&f, (char*)&ehdr, sizeof(ehdr), &off)
            != sizeof(ehdr)) return -1;

    if (!elf_check_support(&ehdr, vfs_path)) return -1;

    // Read program headers
    Elf32_Phdr* phdrs = kmalloc(sizeof(Elf32_Phdr) * ehdr.e_phnum);
    off = ehdr.e_phoff;
    d->inode->i_fop->read(&f, (char*)phdrs, sizeof(Elf32_Phdr) * ehdr.e_phnum, &off);

    // Build a fresh page directory
    PD_t app_pd;
    PDE* pd_mem = (PDE*)PAGE_ADDR(page_alloc_nomap(1));
    app_pd.pde_arr = (PDE*)HHDM_TO_VIRT((uintptr_t)pd_mem);
    pd_init(&app_pd);

    // Load PT_LOAD segments
    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr* ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) continue;

        size_t npages = (ph->p_memsz + PAGE_SIZE - 1) / PAGE_SIZE;
        page_index phys = page_alloc_nomap(npages);
        if (!phys) goto fail;

        // Map into app PD at the segment's virtual address
        for (size_t p = 0; p < npages; p++) {
            pd_map_page(&app_pd,
                ph->p_vaddr + p * PAGE_SIZE,
                (phys + p) * PAGE_SIZE,
                1,
                (ph->p_flags & PF_W) ? 1 : 0,
                1);   // user=1
        }

        // We need a kernel-accessible window to write the segment data.
        // Temporarily vmap the physical pages.
        page_index kwin = vmap(phys, npages, PAGE_FLAG_RW);
        void* kptr = (void*)PAGE_ADDR(kwin);

        memset(kptr, 0, npages * PAGE_SIZE);

        off = ph->p_offset;
        d->inode->i_fop->read(&f, (char*)kptr, ph->p_filesz, &off);

        // Unmap the kernel window (optional if you're tight on vspace)
        for (size_t p = 0; p < npages; p++) unmap_page(kwin + p);
    }

    kfree(phdrs);
    d->inode->i_fop->release(d->inode, &f);

    Linked_PCB_t* pcb = us_task_start((void*)ehdr.e_entry, (char*)vfs_path, app_pd);
    return pcb ? pcb->pid : -1;

fail:
    kfree(phdrs);
    return -1;
}