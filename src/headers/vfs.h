#pragma once
#include <stddef.h>
#include <stdint.h>



#include "arch_types.h"
#include "compiler_defs.h"
#include "super_block.h"
#include "path.h"

#include "types.h"

struct file {
    struct inode *f_inode;          // points to the file/directory inode
    loff_t f_pos;                   // current file position (for read/write)
    unsigned int f_flags;           // O_RDONLY, O_WRONLY, etc.
    void *private_data;             // filesystem-specific data
    const struct file_operations *f_op; // pointer to file operations
};


struct file_operations {
    // struct module *owner; later waybe but commented out to remember
    int (*open) (struct inode *, struct file *);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    int (*release) (struct inode *, struct file *);
};

struct inode_operations{
    struct dentry *(*lookup)(struct inode *, struct dentry *, unsigned int);
    int (*mkdir)(struct inode *, struct dentry *, umode_t);
    int (*rmdir)(struct inode *, struct dentry *);
    int (*create)(struct inode *, struct dentry *, umode_t, bool);
    int (*unlink)(struct inode *, struct dentry *);

    int (*getattr)(const struct path *, struct kstat *, uint32_t, unsigned int);
    int (*setattr)(struct dentry *, struct iattr *);

    // int (*readlink)(struct dentry *, char __user *, int);
    // int (*symlink)(struct inode *, struct dentry *, const char *);
};

struct vfsmount {
    struct dentry *mnt_root;
    struct super_block *mnt_sb;
    int mnt_flags;
};

struct mount {
    struct mount *mnt_parent;
    struct dentry *mnt_mountpoint;
    struct vfsmount mnt;
    struct list_head mnt_mounts;
    struct list_head mnt_child;
};



struct inode {
    // info
    umode_t    i_mode;
    // kuid_t     i_uid;
    // kgid_t     i_gid;
    // unsigned int i_nlink;

    // size nd metadata
    loff_t     i_size;
    time64_t   i_atime;
    time64_t   i_mtime;
    time64_t   i_ctime;

    // dev 
    dev_t      i_rdev;

    // VFS funcs
    const struct inode_operations *i_op;
    const struct file_operations  *i_fop;

    struct super_block *i_sb;

    // type specif data
    void *i_private;
};

struct dentry {
    const char *name;        // filename
    struct dentry *parent;   // parent directory
    struct inode *inode;     // target (NULL = negative dentry)

    struct hlist_node d_sib;      // child of parent list
    struct hlist_head d_children;
};

