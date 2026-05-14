#include "initrd.h"
#include "Logger.h"
#include "bootloader.h"
#include "drivers.h"
#include "err_codes.h"
#include "memory.h"
#include "paging.h"
#include "fs.h"
#include "vfs.h"
#include "vfs/ramfile.h"
#include "config.h"
#include <stdint.h>


struct tar_header *tar_file_headers[32];


int initrd_init() {
    bootloader_loaded_module* initrd = get_bootloader_module("initrd");
    if(!initrd) Sys_Error("Initrd not found\n");

    page_index initrd_start_page = ADDR_TO_PAGE(initrd->mod_start);
    page_index initrd_pages_amount = (initrd->mod_end - initrd->mod_start + PAGE_SIZE - 1) / PAGE_SIZE;

    uintptr_t offset = initrd->mod_start & (PAGE_SIZE - 1);

    page_index new_page = vmap(initrd_start_page, initrd_pages_amount, PAGE_FLAG_RW | PAGE_FLAG_KERNEL);

    char* initrd_data = (char*)(PAGE_ADDR(new_page) + offset);

#if INITRD_DEBUG
    Sys_log("[initrd vaddr]%p\n",initrd_data);
    Sys_log("[initrd data]%s\n",initrd_data);
    Sys_log("mod_start=%p mod_end=%p size=%p\n",
        initrd->mod_start,
        initrd->mod_end,
        initrd->mod_end - initrd->mod_start);
#endif

        uintptr_t size = initrd->mod_end - initrd->mod_start;
    
    parse_tar((uintptr_t)initrd_data);
    
    int res = kpath_mkdir(root_dentry->inode, "/initrd", 0777);
    if(res < 0){

        Sys_Error("Failed to create initrd directory: %d\n", res);

        return res;
    }
    


    struct tar_header* header = tar_file_headers[0];
    struct dentry* dir_dentry = kpath_lookup(root_dentry->inode, "/initrd/");
    if(dir_dentry == NULL) {
#if INITRD_DEBUG
        Sys_Error("Failed to lookup initrd directory\n");
#endif
        return -E_NOENT;
    }

    for(int i = 0; header != NULL; header = tar_file_headers[++i]) {
#if INITRD_DEBUG        
        Sys_log("File: %s, size: %d\n", header->filename, get_tar_size(header->size));
#endif        
        struct dentry* new_dentry = kmalloc(sizeof(struct dentry));
        new_dentry->name = header->filename;
        new_dentry->parent = dir_dentry;
        new_dentry->inode = NULL;

        dir_dentry->inode->i_op->create(
            dir_dentry->inode,
            new_dentry,
            0777, 
            true
        );

        struct ramfile_info* info = kmalloc(sizeof(struct ramfile_info));
        info->start = (uintptr_t)initrd_data + ((size_t)header - (size_t)initrd_data) + 512;
        info->size = get_tar_size(header->size);
        info->fixed_size = true;

        
        new_dentry->inode->i_private = info;
        new_dentry->inode->i_fop = &ram_file_fops;
    }
}
REGISTER_DRIVER_FS(initrd, initrd_init);




unsigned int get_tar_size(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

int parse_tar(uintptr_t address)
{

    int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->filename[0] == '\0')
            break;

        unsigned int size = get_tar_size(header->size);

        tar_file_headers[i] = header;

        address += ((size / 512) + 1) * 512;

        if (size % 512)
            address += 512;

    }

    return i;

}