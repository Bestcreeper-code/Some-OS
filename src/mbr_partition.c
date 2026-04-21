#include "mbr_partition.h"
#include "Logger.h"
#include "blkdev.h"
#include "err_codes.h"
#include "fs.h"
#include "helpers.h"
#include "memory.h"
#include "symbols.h"
#include "types.h"
#include "vfs.h"
#include <stdint.h>
#include <string.h>



int scan_disk_mbr_vfs(struct block_device* blkdev) {
    mbr_layout* mbr = kmalloc(sizeof(mbr_layout));
    RET_IF(!mbr, E_NOMEM);
    memset(mbr, 0, 512);
    
    if (blkdev->ops->read(blkdev, mbr, sizeof(mbr_layout), 0) != sizeof(mbr_layout)) return -E_IO;


    

    Sys_log_NoPos("mbr:{\n");
    for (int i = 0; i < 512; i++) {
        Sys_log_NoPos("%02x", ((uint8_t*)mbr)[i]);
        if ((i & 15) == 15)
            Sys_log_NoPos("\n");
    }
    Sys_log_NoPos("\n} mbr end\n");

    for(int i = 0; i<4;i++ ) {
        Sys_log("scaning /sys/devices/block/%s mbr part %d\n", blkdev->name, i);
        mbr_partition_entry* partit = &mbr->partition_table[i];

        if(partit->total_sectors == 0) continue;

        Sys_log(" /sys/devices/block/%s mbr part %d exists\n", blkdev->name, i);
        char tmp_buff[64];
        
        sprintf(tmp_buff, "/sys/devices/block/%s/%sp%d", blkdev->name, blkdev->name, i+1);
        

        kpath_create(root_dentry->inode, tmp_buff, S_IFBLK | 0660, true);

    }
}