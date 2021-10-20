/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <string.h>

extern info_t *info;

void print_segment(seg_desc_t *seg) {
  uint32_t address = seg->base_1 | (seg->base_2 << 16) | (seg->base_3 << 24);
  uint32_t limit = (seg->limit_1 | (seg->limit_2 << 16));
  uint32_t last_address;
  if (seg->g) {
    last_address = address + (limit + 1) * 4096 - 1;
  } else {
    last_address = address + limit;
  }

  printf("| 0x%8x - 0x%8x [0x%x] ", address, last_address, limit);
  printf("| %d\t ", seg->type);
  printf("| %d\t ", seg->p);
  printf("| %d\t |\n", seg->dpl);
}

void print_gdt(gdt_reg_t gdtr) {
  printf("GDTR address: 0x%8x\n", gdtr);
  seg_desc_t *fseg = gdtr.desc;
  seg_desc_t *iterator = gdtr.desc;

  printf("GDT content:\n");
  printf(
      "+--------------------------------------+---------+-------+-------+\n");
  printf(
      "| Address                              | Type    | Pres  | DPL   |\n");
  printf(
      "+--------------------------------------+---------+-------+-------+\n");
  while (iterator <= fseg + gdtr.limit / sizeof(seg_desc_t)) {
    if (iterator->p) {
      print_segment(iterator);
      printf(
          "+--------------------------------------+---------+-------+-------+"
          "\n");
    }
    iterator++;
  }
}

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
  seg->dpl = SEG_SEL_KRN;
}

void tp() {
  gdt_reg_t gdtr;
  get_gdtr(gdtr);
  print_gdt(gdtr);

  gdt_reg_t new_gdt;
  const int NUMBER_OF_SEGMENTS = 4;
  new_gdt.limit = sizeof(seg_desc_t) * NUMBER_OF_SEGMENTS - 1;
  new_gdt.desc = (seg_desc_t *)0x1000;

  set_null_seg(new_gdt.desc);

  seg_desc_t *code_seg = new_gdt.desc + 1;
  init_seg(code_seg);
  code_seg->limit_1 = (uint16_t)0xffff;
  code_seg->limit_2 = 0xa;
  code_seg->type = SEG_DESC_CODE_XR;

  seg_desc_t *data_seg = code_seg + 1;
  init_seg(data_seg);
  data_seg->limit_1 = (uint16_t)0xffff;
  data_seg->limit_2 = 0x0;
  data_seg->type = SEG_DESC_DATA_RW;

  data_seg = data_seg + 1;
  init_seg(data_seg);
  data_seg->base_2 = 0x60; // Base 0x600000
  data_seg->limit_1 = (uint16_t)0x20;
  data_seg->limit_2 = 0x0;
  data_seg->type = SEG_DESC_DATA_RW;

  
  set_gdtr(new_gdt);
  
  set_cs(1 << 3);
  set_ds(2 << 3);

  get_gdtr(gdtr);
  print_gdt(gdtr);

  char src[64];
  char *dst = 0;
  int i;
  
  memset(src, 0xff, 64);
  for(i = 0; i < 64; i++){
    printf("%c ", src[i]);
  }
  printf("\n");

  set_es(3 << 3);
  _memcpy8(dst, src, 64);
  for(i = 0; i < 64; i++) {
    printf("%c ", dst[i]);
  }

}
