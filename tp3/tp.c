/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <string.h>

#define NUMBER_OF_SEGMENT 6
#define SEG_CODE_R0 1
#define SEG_DATA_R0 2
#define SEG_CODE_R3 3
#define SEG_DATA_R3 4
#define SEG_TSS 5

#define RING_0 0
#define RING_3 3

extern info_t *info;
tss_t tss;

void set_null_seg(seg_desc_t *seg) {
  uint8_t *seg_desc_bytes = (uint8_t *)seg;
  for (int i = 0; i < 8; i++) {
    *(seg_desc_bytes + i) = 0;
  }
}

void init_seg(seg_desc_t *seg) {
  set_null_seg(seg);
  seg->p = 0b1;
  seg->g = 0b1;
  seg->d = 0b1;
  seg->s = 0b1;
}

void set_seg_r0(seg_desc_t *seg) { seg->dpl = SEG_SEL_KRN; }

void set_seg_r3(seg_desc_t *seg) { seg->dpl = SEG_SEL_USR; }

void gdt_init() {
  // Question 1
  gdt_reg_t gdtr;
  gdtr.limit = sizeof(seg_desc_t) * NUMBER_OF_SEGMENT - 1;
  gdtr.desc = (seg_desc_t *)0x1000;
  set_null_seg(gdtr.desc);
  set_gdtr(gdtr);
}

void gdt_add_segment(uint16_t type, uint16_t dpl) {
  static int seg_rank = 1;
  gdt_reg_t gdtr;
  get_gdtr(gdtr);
  seg_desc_t *new_seg = gdtr.desc + seg_rank;
  init_seg(new_seg);
  new_seg->limit_1 = (uint16_t)0xffff;
  new_seg->limit_2 = 0xf;
  new_seg->type = type;
  if (dpl == RING_0) {
    set_seg_r0(new_seg);
  } else if (dpl == RING_3) {
    set_seg_r3(new_seg);
  }
  seg_rank++;
}

void gdt_add_tss() {
  gdt_reg_t gdtr;
  get_gdtr(gdtr);

  int seg_rank = NUMBER_OF_SEGMENT - 1; // TSS desc is the last one
  seg_desc_t *tss_seg = gdtr.desc + seg_rank;
  init_seg(tss_seg);
  
  uint32_t base = (uint32_t) &tss;
  uint32_t limit = sizeof(tss);
  set_null_seg(tss_seg);
  tss_seg->limit_1 = limit;
  tss_seg->limit_2 = (limit & (0xf << 16)) >> 16;
  tss_seg->base_1 = base;
  tss_seg->base_2 = (base & (0xf << 16)) >> 16;
  tss_seg->base_3 = (base & (0xf << 24)) >> 24;
  tss_seg->type = SEG_DESC_SYS_TSS_AVL_32;
  tss_seg->p = 0b1;
}

// Question 2
void userland() { 
  //asm volatile("mov %cr0, %eax");
  printf("Toto ring 3\n");
}

void tp() {
  gdt_init();
  gdt_add_segment(SEG_DESC_CODE_XR, RING_0);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_0);
  gdt_add_segment(SEG_DESC_CODE_XR, RING_3);
  gdt_add_segment(SEG_DESC_DATA_RW, RING_3);
  gdt_add_tss();

  set_cs(gdt_seg_sel(SEG_CODE_R0, RING_0));
  set_ss(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_ds(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_fs(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_es(gdt_seg_sel(SEG_DATA_R0, RING_0));
  set_gs(gdt_seg_sel(SEG_DATA_R0, RING_0));

  printf("Toto ring 0\n");

  // Question 3
  // Question 3.1 -> No error
  set_ds(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_es(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_fs(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_gs(gdt_seg_sel(SEG_DATA_R3, RING_3));

  memset(&tss, 0, sizeof(tss));
  tss.s0.esp = get_ebp();
  tss.s0.ss = gdt_seg_sel(SEG_DATA_R0, RING_0);
  set_tr(gdt_seg_sel(SEG_TSS, RING_0));

  printf("Userland fct @0x%x\n", &userland);
  
  // Question 3.2 -> fatal error
  //set_ss(gdt_seg_sel(SEG_DATA_R3, RING_3));

  // Question 3.3 -> fatal error
  //set_cs(gdt_seg_sel(SEG_CODE_R3, RING_3));
  //farjump...

  // Question 3.4 
  void * usrlnd_ptr = userland;
  asm volatile("movl %esp, %eax");
  asm volatile("push %0"::"i"(gdt_seg_sel(SEG_DATA_R3, RING_3)));
  asm volatile("push %eax");
  asm volatile("pushf");
  asm volatile("push %0"::"i"(gdt_seg_sel(SEG_CODE_R3, RING_3)));
  asm volatile("push %%ebx"::"b"(usrlnd_ptr)); 
  asm volatile("iret");
}
