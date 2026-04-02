#include "blkdev.h"
#include "Logger.h"
#include "config.h"
#include "container_of.h"
#include "fs.h"
#include "io.h"
#include "memory.h"
#include "helpers.h"
#include "types.h"
#include "vfs.h"
#include <stddef.h>




size_t _blkdev_id_map[128/ (sizeof(size_t)*8)];

struct list_head* block_device_list_start;
short block_device_amount;

struct block_device* Register_Block_Device(const char *name, lsize_t size,
    size_t block_size, struct block_device_ops ops, void *private_data) {
    
#if BLKDEV_DEBUG
    Sys_log("Registered blkdev [%s] with 0x%llx bytes\n",
        name,
        size);
#endif

    struct block_device* blkdev = kmalloc(sizeof(struct block_device));
    if(!blkdev)return NULL;

    blkdev->name = name;
    blkdev->size = size;
    blkdev->block_size = block_size;
    blkdev->ops = ops;
    blkdev->private_data = private_data;
    blkdev->id = wbitmap_alloc_first((char*)_blkdev_id_map, sizeof(_blkdev_id_map));

    if(!block_device_list_start){
        block_device_list_start = &blkdev->list;
        blkdev->list.next = NULL;
        blkdev->list.prev = &blkdev->list;//ez way to get the last one

        block_device_amount++;
        kpath_create(root_dentry->inode, "/dev/", name, S_IFBLK , false);
        return blkdev;
    }

    struct list_head* end_entry = block_device_list_start->prev;
    
    RET_IF(end_entry == 0, NULL);
    
    end_entry->next = &blkdev->list;
    blkdev->list.prev = end_entry;
    blkdev->list.next = NULL;

    block_device_list_start->prev = &blkdev->list;

    block_device_amount++;
    kpath_create(root_dentry->inode, "/dev/", name, S_IFBLK , false);
    return blkdev;
}

int Unregister_Block_Device(struct block_device* blkdev){
    RET_IF(!blkdev, -1);

    if (&blkdev->list == block_device_list_start) {
        block_device_list_start = blkdev->list.next;
        if (block_device_list_start)
            block_device_list_start->prev = blkdev->list.prev;
    } else {
        blkdev->list.prev->next = blkdev->list.next;
        if (!blkdev->list.next)
            block_device_list_start->prev = blkdev->list.prev;
        else
            blkdev->list.next->prev = blkdev->list.prev;
    }

    bitmap_free_bit((char*)_blkdev_id_map, blkdev->id);
    kfree(blkdev);
    return 0;
}

const struct block_device* Get_Block_Device(int id) {
    RET_IF(id < 0, NULL);

    struct list_head* current = block_device_list_start;
    while (current) {
        struct block_device* blkdev = container_of(current, struct block_device, list);
        
        RET_IF(blkdev->id == id, blkdev);
        
        current = current->next;
    }
    return NULL; 
}

int Get_Block_Device_Amount(){
    return block_device_amount;
}
