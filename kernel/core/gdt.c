#include <gdt.h>
#include <default.h>
#include <segmem.h>
#include <debug.h>
#include <segment.h>

void gdt_init() {
  gdt_reg_t gdtr;
  gdtr.limit = sizeof(seg_desc_t) * NUMBER_OF_SEGMENT - 1;
  gdtr.desc = (seg_desc_t *)GDT_ADDR;
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

void gdt_add_tss(tss_t *tss) {
  gdt_reg_t gdtr;
  get_gdtr(gdtr);

  int tss_rank = NUMBER_OF_SEGMENT - 1; // TSS desc is the last one
  seg_desc_t *tss_seg = gdtr.desc + tss_rank;

  uint32_t base = (uint32_t) tss;
  uint32_t limit = sizeof(*tss);
  set_null_seg(tss_seg);
  tss_seg->limit_1 = limit;
  tss_seg->limit_2 = (limit & (0xf << 16)) >> 16;
  tss_seg->base_1 = base;
  tss_seg->base_2 = (base & (0xf << 16)) >> 16;
  tss_seg->base_3 = (base & (0xf << 24)) >> 24;
  tss_seg->type = SEG_DESC_SYS_TSS_AVL_32;
  tss_seg->p = 0b1;
}

void gdt_print() {
  gdt_reg_t gdtr;
  get_gdtr(gdtr);

  seg_desc_t *fseg = gdtr.desc;
  seg_desc_t *iterator = gdtr.desc;

  debug(
      "+--------------------------------------+---------+-------+-------+\n");
  debug(
      "| Address   [GDT at 0x%8x]        | Type    | Pres  | DPL   |\n", gdtr.desc);
  debug(
      "+--------------------------------------+---------+-------+-------+\n");
  while (iterator <= fseg + gdtr.limit / sizeof(seg_desc_t)) {
    if (iterator->p) {
      print_seg(iterator);
      debug(
          "+--------------------------------------+---------+-------+-------+"
          "\n");
    }
    iterator++;
  }
}


