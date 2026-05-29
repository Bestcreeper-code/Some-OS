#include "paging.h"



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
