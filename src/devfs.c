#include "container_of.h"
#include "fs.h"
#include "memory.h"
#include "types.h"
#include "vfs.h"
#include <string.h>




int devfs_init(){
    kpath_mkdir(root_dentry->inode, "/", "dev/", 0755);
}