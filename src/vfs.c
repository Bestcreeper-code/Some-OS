#include "vfs.h"
#include "container_of.h"
#include "fs.h"
#include "helpers.h"
#include "lists.h"
#include "string.h"
#include "memory.h"
#include "types.h"
#include <asm-generic/errno-base.h>

static struct file_system_type *fs_registry = NULL;

extern struct inode tmp_root_inode;
struct dentry tmp_root_dentry = {
    .inode = &tmp_root_inode,
    .name = "/",
    .parent = NULL
};

struct inode tmp_root_inode = {
    .i_op = &def_vfs_i_ops,
    .i_dentry = &tmp_root_dentry,
    .i_count= 9999,
    .i_mode = S_IFDIR | 0777
    
};

struct dentry* root_dentry =  &tmp_root_dentry;

struct inode_operations def_vfs_i_ops = {
    .create = vfs_create,
    .lookup = vfs_lookup,
    .mkdir = vfs_mkdir,
};



int register_filesystem(struct file_system_type *fs) {
    fs->next = fs_registry;
    fs_registry = fs;
    return 0;
}

struct file_system_type *find_filesystem(const char *name) {
    struct file_system_type *fs = fs_registry;
    while (fs) {
        if (strcmp(fs->name, name) == 0) return fs;
        fs = fs->next;
    }
    return NULL;
}

struct dentry *vfs_lookup(struct inode* dir, struct dentry* file, unsigned int flags) {
    

    struct hlist_node *pos;
    hlist_for_each(pos, &dir->i_dentry->d_children){
        
        struct dentry* curr_sib = container_of(pos, struct dentry, d_sib);

        if (strcmp(file->name, curr_sib->name) == 0){
            return curr_sib;
        }
    }
    return NULL;
}

int vfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl) {
    struct inode *new_inode = kmalloc(sizeof(struct inode));
    RET_IF(!new_inode, -ENOMEM);

    new_inode->i_mode = mode;
    new_inode->i_op   = dir->i_op;
    new_inode->i_fop  = dir->i_fop;

    new_inode->i_dentry  = dentry;
    new_inode->i_count   = 0;
    new_inode->i_private = NULL;
    
    dentry->inode = new_inode;
    
    
    hlist_add_head(&dentry->d_sib, &dir->i_dentry->d_children);
    
    return 0;
}

int vfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode) {

    return vfs_create(dir, dentry, S_IFDIR | (mode & ~S_IFMT), false);
}