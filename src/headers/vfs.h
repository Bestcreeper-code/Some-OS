#pragma once
#include <stddef.h>
#include <stdint.h>



#include "arch_types.h"
#include "compiler_defs.h"

#include "types.h"

struct file {
    struct inode *f_inode;          // points to the file/directory inode
    loff_t f_pos;                   // current file position (for read/write)
    unsigned int f_flags;           // O_RDONLY, O_WRONLY, etc.
    void *private_data;             // filesystem-specific data
    const struct file_operations *f_op; // pointer to file operations
};

#warning struct module *owner to add
struct file_operations {
    // struct module *owner; later
    int (*open) (struct inode *, struct file *);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    int (*release) (struct inode *, struct file *);
};

struct inode_operations{

};




struct inode {
    // info
    umode_t    i_mode;
    kuid_t     i_uid;
    kgid_t     i_gid;
    unsigned int i_nlink;

    // size nd metadata
    loff_t     i_size;
    time64_t   i_atime;
    time64_t   i_mtime;
    time64_t   i_ctime;

    // device 
    dev_t      i_rdev;

    // VFS hooks
    const struct inode_operations *i_op;
    const struct file_operations  *i_fop;

    struct super_block *i_sb;

    // FS-specific or driver-specific data
    void *i_private;
};

struct dentry {
    const char *name;        // filename
    struct dentry *parent;   // parent directory
    struct inode *inode;     // target (NULL = negative dentry)

    struct hlist_node d_sib;      // child of parent list
    struct hlist_head d_children;
};