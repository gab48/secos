/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>

extern info_t *info;

void print_segment(seg_desc_t * seg) {  
  uint32_t address = seg->base_1 | (seg->base_2 << 16) | (seg->base_3 << 24);
  debug("| 0x%x - 0x%x\t ", address, address + (seg->limit_1 | (seg->limit_2 << 16)));
  debug("| %d\t ", seg->type);
  debug("| %d\t ", seg->p);
  debug("| %d\t |\n", seg->dpl);
}

void tp() {
  gdt_reg_t gdtr;
  get_gdtr(gdtr);
  debug("GDTR address: 0x%x\n", gdtr);
  seg_desc_t * fseg = gdtr.desc;
  seg_desc_t * iterator = gdtr.desc;

  printf("GDT content:\n");
  while(iterator <= fseg + gdtr.limit/sizeof(seg_desc_t)) {
    if (iterator->p) {
      print_segment(iterator);
    }
    iterator++;
  }
}
