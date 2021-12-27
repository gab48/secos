#include <cr.h>
#include <debug.h>
#include <page.h>
#include <pagemem.h>
#include <string.h>
#include <types.h>

void enable_paging() {
  cr0_reg_t cr0 = {.raw = get_cr0()};
  cr0.pg = 0b1;
  set_cr0(cr0);
}

void set_pgd_addr(pde32_t *pgd_addr) { set_cr3(pgd_addr); }

void init_krn_page(pde32_t *pgd) {
  memset((void *)pgd, 0, PAGE_SIZE);
  pte32_t *ptb = (pte32_t *)((uint32_t)pgd + 0x1000);
  pte32_t *ptb1 = (pte32_t *)((uint32_t)pgd + 0x2000);
  int i;
  for (i = 0; i < 1024; i++) {
    pg_set_entry(&ptb[i], PG_RW | PG_KRN, i);
    pg_set_entry(&ptb1[i], PG_RW | PG_KRN, i + 1024);
  }
  pg_set_entry(&pgd[0], PG_RW | PG_KRN, page_nr(ptb));
  pg_set_entry(&pgd[1], PG_RW | PG_KRN, page_nr(ptb1));
}

void init_usr_page(pde32_t *pgd, uint32_t base) {
  int i;
  int pgd_idx = pd32_idx(base);
  int ptb_idx = pt32_idx(base);

  pte32_t *ptb1 = (pte32_t *)((uint32_t)pgd + 0x1000);
  memset((void *)ptb1, 0, PAGE_SIZE);
  for (i = 0; i < 1024; i++) {
    pg_set_entry(&ptb1[i], PG_RW | PG_USR, i);
  }
  
  pte32_t *ptb2 = (pte32_t *)((uint32_t)pgd + 0x2000);
  memset((void *)ptb2, 0, PAGE_SIZE);
  for (i = 0; i < 15; i++) {
    pg_set_entry(&ptb2[i + ptb_idx], PG_RW | PG_USR, i + ptb_idx +(pgd_idx<<10));
  }
  pg_set_entry(&ptb2[i + ptb_idx], PG_RW | PG_KRN, i + (pgd_idx<<10));
  
  memset((void *)pgd, 0, PAGE_SIZE);
  pg_set_entry(&pgd[0], PG_RW | PG_USR, page_nr(ptb1));
  pg_set_entry(&pgd[pgd_idx], PG_RW | PG_USR, page_nr(ptb2));
}

void add_page_shared_memory(pde32_t *pgd, uint32_t *virtual, uint32_t *physical) {
  int pgd_idx = pd32_idx(virtual);
  int ptb_idx = pt32_idx(virtual);
  pte32_t *ptb = (pte32_t *)((uint32_t)pgd + 0x3000);
  pg_set_entry(&ptb[ptb_idx], PG_RW | PG_USR, page_nr(physical));
  pg_set_entry(&pgd[pgd_idx], PG_RW | PG_USR, page_nr(ptb));
}

void pgd_show(pde32_t *pgd) {
  uint32_t i;
  if (pgd != NULL) {
    debug("+--PGD (0x%8x)--+\n", pgd);
    for (i = 0; i < PDE32_PER_PD; i++) {
      if (pgd[i].p == 1) {
        uint32_t high_addr = i << 10;
        pte32_t *ptb_addr = (pte32_t *)(pgd[i].addr << 12);
        debug("  %d: ptb_addr = 0x%8x dpl=%d\n", i, ptb_addr, pgd[i].lvl);
        ptb_show(ptb_addr, high_addr);
      }
    }
    debug("+---END PGD ---+\n\n");
  }
}

void ptb_show(pte32_t *ptb, uint32_t high_addr) {
  uint32_t i;
  if (ptb != NULL) {
    for (i = 0; i < PTE32_PER_PT; i++) {
      if (ptb[i].p == 1) {
        uint32_t virt_addr = (high_addr | i) << 12;
        debug("    - %d: phy=0x%8x virt=0x%8x dpl=%d\n", i, ptb[i].addr << 12, virt_addr, ptb[i].lvl);
      }
    }
  }
}

