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
pde32_t *pgd_krn = (pde32_t *)PGD_KRN_ADDR;
pde32_t *pgd_user1 = (pde32_t *)PGD_US1_ADDR;
pde32_t *pgd_user2 = (pde32_t *)PGD_US2_ADDR;

typedef struct counter_type {
  uint32_t *value;
  uint32_t *mutex;
} __attribute__((packed)) counter_t;

__attribute__((section(".user2"))) void sys_counter(uint32_t *counter) {
  asm volatile("int $0x80" ::"a"(counter));
}

__attribute__((section(".user1"))) void user1() {
  counter_t counter;
  counter.value = (uint32_t *) SHRD_MEM_VIRT_US1_ADDR;
  counter.mutex = (uint32_t *)(SHRD_MEM_VIRT_US1_ADDR + sizeof(uint32_t));
  *counter.value = 0;
  *counter.mutex = 0;
  while (true) {
    if (*counter.mutex == 0) {
    *counter.value += 1;
    *counter.mutex = 1;
    }
  }
}

__attribute__((section(".user2"))) void user2() {
  counter_t counter;
  counter.value = (uint32_t *) SHRD_MEM_VIRT_US2_ADDR;
  counter.mutex = (uint32_t *)(SHRD_MEM_VIRT_US2_ADDR + sizeof(uint32_t));
  while (true) {
    if (*counter.mutex == 1) {
      sys_counter(counter.value);
      *counter.mutex = 0;
    }
  }
}

void setup_segmentation() {
  gdt_init();
  gdt_add_segment(SEG_DESC_CODE_XR, RING_0);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_0);
  gdt_add_segment(SEG_DESC_CODE_XR, RING_3);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_3);
  gdt_add_tss(&tss);
  init_tss(&tss);
  //gdt_print();
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
  set_handler(IDT_IRQ0_IDX, SEG_CODE_R0, RING_0, SEG_DESC_SYS_INTR_GATE_32, task_scheduler);
  set_handler(IDT_PRINT_HANDLER_IDX, SEG_CODE_R0, RING_3, SEG_DESC_SYS_TRAP_GATE_32, print_isr);
}

void setup_paging() {
  init_krn_page(pgd_krn);
  init_usr_page(pgd_user1, USER1_SECTION_ADDR);
  init_usr_page(pgd_user2, USER2_SECTION_ADDR);
  
  add_page_shared_memory(pgd_user1, (uint32_t *)SHRD_MEM_VIRT_US1_ADDR, (uint32_t *)SHRD_MEM_PHY_ADDR);
  add_page_shared_memory(pgd_user2, (uint32_t *)SHRD_MEM_VIRT_US2_ADDR, (uint32_t *)SHRD_MEM_PHY_ADDR);
  
  set_cr3(pgd_krn);
  //pgd_show(pgd_krn);
  enable_paging();
}

void setup_tasks() {
  tasks_init(&tss);
  load_task(1, pgd_user1, user1, STACK_R0_US1, STACK_R3_US1);
  load_task(2, pgd_user2, user2, STACK_R0_US2, STACK_R3_US2);
}

void start_os() {
  force_interrupts_on();
  while(true);
}


void tp() {
  setup_segmentation();
  setup_interruptions();
  setup_paging();
  setup_tasks();

  start_os();
}
