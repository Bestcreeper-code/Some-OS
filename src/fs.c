#include "fs.h"
#include "Logger.h"
#include "container_of.h"
#include "err_codes.h"
#include "helpers.h"
#include "memory.h"
#include "string.h"
#include "types.h"
#include "vfs.h"
#include <stdbool.h>
#include "io.h"



struct dentry* kpath_lookup(struct inode* start, const char* path) {
    if (!path || !start) return NULL;

    char* path_copy = strdup(path);
    if (!path_copy) return NULL;

    struct dentry* curr = start->i_dentry;
    char* save  = NULL;
    char* token = strtok_r(path_copy, "/", &save);

    while (token) {

        if (strcmp(token, ".") == 0) {
            token = strtok_r(NULL, "/", &save);
            continue;
        }

        if (strcmp(token, "..") == 0) {
            curr  = curr->parent ? curr->parent : curr;
            token = strtok_r(NULL, "/", &save);
            continue;
        }

        struct dentry next_tmp = { .name = token, .inode = NULL, .parent = curr };

        struct dentry* result = curr->inode->i_op->lookup(curr->inode, &next_tmp, 0);
        if (!result) {
            kfree(path_copy);
            return NULL;
        }

        curr  = result;
        token = strtok_r(NULL, "/", &save);
    }

    kfree(path_copy);
    curr->inode->i_count++;
    return curr;
}




int kpath_create(struct inode* start, const char* path, char* name, umode_t mode, bool excl){
    struct dentry* dir = kpath_lookup(start, path);

    if(!dir) RET_ERR(E_NOENT);
    
    struct dentry* newdir = kmalloc(sizeof(struct dentry));
    if(!newdir) RET_ERR(E_NOMEM);

    newdir->name = name;
        
    

    return dir->inode->i_op->create(dir->inode, newdir, mode, excl);
}


int kpath_mkdir(struct inode* start, const char* path, char* name, umode_t mode) {
    return kpath_create(start, path, name, mode | S_IFDIR, false);
}

int kpath_rmdir(struct inode* start, const char* path, char* name) {
    struct dentry* dir = kpath_lookup(start, path);

    if (!dir) RET_ERR(E_NOENT);

    struct dentry* target = kmalloc(sizeof(struct dentry));
    if (!target) RET_ERR(E_NOMEM);

    target->name   = name;
    target->parent = dir;

    return dir->inode->i_op->rmdir(dir->inode, target);
}

int path_unlink(struct inode* start, const char* path, char* name) {
    struct dentry* dir = kpath_lookup(start, path);

    if (!dir) RET_ERR(E_NOENT);

    struct dentry* target = kmalloc(sizeof(struct dentry));
    if (!target) RET_ERR(E_NOMEM);

    target->name   = name;
    target->parent = dir;

    return dir->inode->i_op->unlink(dir->inode, target);
}

void tree(struct dentry* d, int depth){
    if(!d) return;
    serial_write_string("||");
    for(int i=0;i<depth-1;i++) serial_write_string("--");
    serial_write_string(d->name);
    serial_write_string("\n");

    struct hlist_node* pos;
    struct dentry* child;

    hlist_for_each(pos, &d->d_children){
        child = container_of(pos, struct dentry, d_sib);
        tree(child, depth+1);
    }
}



/*
int (*mkdir)(struct inode *, struct dentry *, umode_t);
    int (*rmdir)(struct inode *, struct dentry *);
    int (*create)(struct inode *, struct dentry *, umode_t, bool);
    int (*unlink)(struct inode *, struct dentry *);
*/