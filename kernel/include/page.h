#ifndef __PAGE_H__
#define __PAGE_H__

#include <pagemem.h>
#include <types.h>

void enable_paging();
void set_pgd_addr(pde32_t *pgd_addr);
void init_krn_page(pde32_t *pgd);
void init_usr_page(pde32_t *pgd, uint32_t base);
void add_page_shared_memory(pde32_t *pgd, uint32_t* virtual, uint32_t* physical);
void ptb_show(pte32_t *ptb, uint32_t high_addr);
void pgd_show(pde32_t *pgd);

#endif
