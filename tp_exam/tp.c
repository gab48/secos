/* GPLv2 (c) Airbus */
#include <debug.h>
#include <default.h>
#include <gdt.h>
#include <handlers.h>
#include <info.h>
#include <intr.h>
#include <page.h>
#include <pagemem.h>
#include <segmem.h>
#include <segment.h>
#include <tasks.h>
#include <tss.h>
#include <asm.h>
#include <cr.h>

extern info_t *info;
tss_t tss;
pde32_t *pgd_krn = (pde32_t *)0x800000;
pde32_t *pgd_user1 = (pde32_t *)0x810000;
pde32_t *pgd_user2 = (pde32_t *)0x820000;

__attribute__((section(".user1"))) void user1() {
  debug("user1");
  //while (1) {
  //}
}

__attribute__((section(".user2"))) void user2() {
  debug("user2");
  //asm volatile("int $0x80" ::"S"("Hello\n"));
  while (1) {
  }
}

void enable_segmentation() {
  gdt_init();
  gdt_add_segment(SEG_DESC_CODE_XR, RING_0);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_0);
  gdt_add_segment(SEG_DESC_CODE_XR, RING_3);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_3);
  gdt_add_tss(&tss);
  init_tss(&tss);
  gdt_print();
  set_cs(gdt_seg_sel(SEG_CODE_R0, RING_0));
  set_ss(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_ds(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_fs(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_es(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_gs(gdt_seg_sel(SEG_DATA_R0, RING_0));
}

void setup_interruptions() {
  intr_init();
  idt_reg_t idtr;
  get_idtr(idtr);
  set_handler(IDT_IRQ0_IDX, SEG_CODE_R0, RING_0, task_scheduler);
  set_handler(IDT_PRINT_HANDLER_IDX, SEG_CODE_R0, RING_0, print_isr);
}

void getting_to_ring3() {
  set_ds(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_es(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_fs(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_gs(gdt_seg_sel(SEG_DATA_R3, RING_3));
}

void paging() {
  init_krn_page(pgd_krn);
  init_usr_page(pgd_user1, 0x400000);
  init_usr_page(pgd_user2, 0x410000);
  
  add_page_translation(pgd_user1, (uint32_t *)0x10000, (uint32_t *)0x600000);
  add_page_translation(pgd_user1, (uint32_t *)0x302010, (uint32_t *)0x302010);
  add_page_translation(pgd_user2, (uint32_t *)0x20000, (uint32_t *)0x600000);
  add_page_translation(pgd_user2, (uint32_t *)0x302010, (uint32_t *)0x302010);
  
  set_cr3(pgd_krn);
  enable_paging();
}

void tp() {
  enable_segmentation();
  setup_interruptions();
  paging();
  getting_to_ring3();

  tasks_init(&tss);
  load_task(1, pgd_user1, 0x40ffff, 0x40ffff, user1);
  load_task(2, pgd_user2, 0x41ffff, 0x41ffff, user2);

  force_interrupts_on();
  while(1){}
}
