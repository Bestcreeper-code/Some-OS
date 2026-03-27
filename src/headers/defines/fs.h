#pragma once

#include "lists.h"
#include "types.h"
#include "compiler_defs.h"


#define FS_INIT_FUNCS_SECTION _GCC_SECTION("fs_drivers_list") 


struct file_system_type {
    const char *name;
    struct super_block *(*mount)(struct file_system_type *, int, const char *, void *);
    void (*kill_sb)(struct super_block *);
    struct file_system_type * next;
    struct list_head fs_supers;
};