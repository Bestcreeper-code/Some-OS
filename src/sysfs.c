#include "container_of.h"
#include "fs.h"
#include "memory.h"
#include "types.h"
#include "vfs.h"
#include <string.h>




int sysfs_init(){
    kpath_mkdir(root_dentry->inode, "/sys", 0555);
    kpath_mkdir(root_dentry->inode, "/sys/devices", 0755);    
}