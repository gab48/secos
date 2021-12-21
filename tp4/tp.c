/* GPLv2 (c) Airbus */
#include <cr.h>
#include <debug.h>
#include <info.h>
#include <pagemem.h>

extern info_t *info;

// Question 1
void print_cr3() {
  uint32_t cr3;
  asm volatile("mov %%eax, %%cr3" : "=a"(cr3)::);
  debug("CR3 => 0x%x\n", cr3);
}

void enable_paging(pde32_t *pgd_addr) {
  // Question 3: Set cr0 to enable paging
  cr0_reg_t cr0 = {.raw = get_cr0()};
  cr0.pg = 0b1;

  // Question 2: Load PGD address in cr3 registry
  set_cr3(pgd_addr);

  set_cr0(cr0);
}

void tp() {
  int i;
  print_cr3();

  pde32_t *pgd = (pde32_t *)0x600000;
  pte32_t *ptb_1 = (pte32_t *)0x601000;
  pte32_t *ptb_2 = (pte32_t *)0x602000;
  pte32_t *ptb_3 = (pte32_t *)0x603000;

  for (i = 0; i < 1024; i++) {
    pg_set_entry(&ptb_1[i], PG_RW | PG_KRN, i);
    pg_set_entry(&ptb_2[i], PG_RW | PG_KRN, i + 1024);
  }

  memset((void *)pgd, 0, PAGE_SIZE);

  pg_set_entry(&pgd[0], PG_RW | PG_KRN, page_nr(ptb_1));
  pg_set_entry(&pgd[1], PG_RW | PG_KRN, page_nr(ptb_2));

  debug("PGD => 0x%8x\n", pgd->addr);

  enable_paging(pgd);

  debug("PTB[0] => %p\n", ptb_2[0].d);
  ptb_2[0].d=0b1;
  debug("PTB[0] => %p\n", ptb_2[0].d);

  uint32_t *target = (uint32_t*)0xc0000000;
  int pgd_idx = pd32_idx(target);
  int ptb_idx = pt32_idx(target);

  memset((void*)ptb_3, 0, PAGE_SIZE);
  pg_set_entry(&ptb_3[ptb_idx], PG_RW | PG_KRN, page_nr(pgd));
  pg_set_entry(&pgd[pgd_idx], PG_RW | PG_KRN, page_nr(ptb_3));

  debug("PGD[0] => 0x%8x | virtual = 0x%8x\n", pgd[0].raw, *target);

  char *v1 = (char*)0x700000;
  char *v2 = (char*)0x7ff000;

  ptb_idx = pt32_idx(v1);
  pg_set_entry(&ptb_2[ptb_idx], PG_RW | PG_KRN, 2);
  ptb_idx = pt32_idx(v2);
  pg_set_entry(&ptb_2[ptb_idx], PG_RW | PG_KRN, 2);

  debug("%p = %s | %p = %s\n", v1, v1, v2, v2);

  *target = 0;

}
