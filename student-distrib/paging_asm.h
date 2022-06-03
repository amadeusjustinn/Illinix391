#ifndef PAGING_ASM_H
#define PAGING_ASM_H

#include "lib.h"

/* Loads CR3 with the address of the page directory */
extern void load_page_dir(uint32_t *); // cr3

/**
 * Enables page size extension (PSE) for 4 MiB pages,
 * and sets the paging (PG) and protection (PE) bits of CR0
 */
extern void enable_paging(void); // cr4 then cr0

#endif
