#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#define PAGE_SIZE       4096
#define PAGE_MASK       (~(PAGE_SIZE - 1))


#define PAGE_ALIGN_DOWN(addr)  ((uintptr_t)(addr) & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)    (((uintptr_t)(addr) + PAGE_SIZE - 1) & PAGE_MASK)


#define PAGE_IS_ALIGNED(addr)  (((uintptr_t)(addr) & ~PAGE_MASK) == 0)

#define PAGE_ADDR(idx)         ((uintptr_t)((idx) << 12))

#define ADDR_TO_PAGE(addr)     (((page_index)(addr)) >> 12)




typedef uintptr_t page_index;
typedef uintptr_t page_table_index;
typedef uintptr_t page_dir_index;

typedef enum {
    PAGE_FLAG_RO   = 0 << 0,
    PAGE_FLAG_RW   = 1 << 0,
    
    PAGE_FLAG_KERNEL = 0 << 1,
    PAGE_FLAG_USER = 1 << 1
} PAGING_FLAG;

typedef struct {
    bool     present;
    uintptr_t index;
    bool     rw;
    bool     us;
    bool     nx;
} PAGE;

int setup_paging();

page_index page_alloc(size_t amount, char flags);
void       page_free(page_index pa, size_t amount);


int  map_page(page_index virt, page_index phys, uint8_t present, uint8_t rw, uint8_t user);
int  unmap_page(page_index virt);
int  map_range(page_index virt, page_index phys, size_t count, char flags); /* bk map */


PAGE  virt_to_page(void *address);
int  page_write(page_index virt, PAGE page);
bool  page_is_present(page_index virt);


void tlb_flush_page(uintptr_t virt);
void tlb_flush_all();

size_t get_used_ram();

page_index vmap(page_index phys, size_t count, char flags);

#endif /* PAGING_H */