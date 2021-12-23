#include <cr.h>
#include <page.h>
#include <pagemem.h>
#include <string.h>
#include <types.h>
#include <debug.h>

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
    pg_set_entry(&ptb1[i], PG_RW | PG_KRN, i+1024);
  }
  pg_set_entry(&pgd[0], PG_RW | PG_KRN, page_nr(ptb));
  pg_set_entry(&pgd[1], PG_RW | PG_KRN, page_nr(ptb1));
}

void init_usr_page(pde32_t *pgd, uint32_t base) {
  memset((void *)pgd, 0, PAGE_SIZE);
  int pgd_idx = pd32_idx(base);
  int ptb_idx = pt32_idx(base);
  debug("Base 0x%8x | pgd_idx = %d | ptb_idx = %d\n", base, pgd_idx, ptb_idx);
  pte32_t *ptb = (pte32_t *)((uint32_t)pgd + 0x1000);
  int i;
  for (i = 0; i < 16; i++) {
    pg_set_entry(&ptb[i+ptb_idx], PG_RW | PG_USR, i+ptb_idx);
  }
  pg_set_entry(&pgd[pgd_idx], PG_RW | PG_USR, page_nr(ptb));

  pte32_t *ptb1 = (pte32_t *)((uint32_t)pgd + 0x2000);
  for (i = 0; i < 1024; i++) {
    pg_set_entry(&ptb1[i], PG_RW | PG_USR, i);
  }
  pg_set_entry(&pgd[0], PG_RW | PG_USR, page_nr(ptb1));

}

void add_page_translation(pde32_t *pgd, uint32_t* virtual, uint32_t* physical) {
  static int counter = 1;
  int pgd_idx = pd32_idx(virtual);
  int ptb_idx = pt32_idx(virtual);
  pte32_t *ptb = (pte32_t *)((uint32_t)pgd + 0x2000*counter);
  memset((void*)ptb, 0, PAGE_SIZE);
  pg_set_entry(&ptb[ptb_idx], PG_RW | PG_KRN, page_nr(physical));
  pg_set_entry(&pgd[pgd_idx], PG_RW | PG_KRN, page_nr(ptb));
  counter++;
}

