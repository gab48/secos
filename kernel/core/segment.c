#include <segment.h>
#include <segmem.h>
#include <debug.h>

void init_seg(seg_desc_t *seg) {
  set_null_seg(seg);
  seg->p = 0b1;
  seg->g = 0b1;
  seg->d = 0b1;
  seg->s = 0b1;
}

void set_null_seg(seg_desc_t *seg) {
  uint8_t *seg_desc_bytes = (uint8_t *) seg;
  int i;
  for (i = 0; i < 8; i++) { *(seg_desc_bytes + i) = 0;}
}

void set_seg_r0(seg_desc_t *seg) { seg->dpl = SEG_SEL_KRN; }
void set_seg_r3(seg_desc_t *seg) { seg->dpl = SEG_SEL_USR; }

void print_seg(seg_desc_t *seg) {
  uint32_t address = seg->base_1 | (seg->base_2 << 16) | (seg->base_3 << 24);
  uint32_t limit = (seg->limit_1 | (seg->limit_2 << 16));
  uint32_t last_address;
  if (seg->g) {
    last_address = address + (limit + 1) * 4096 - 1;
  } else {
    last_address = address + limit;
  }

  debug("| 0x%8x - 0x%8x [0x%x] ", address, last_address, limit);
  debug("| %d\t ", seg->type);
  debug("| %d\t ", seg->p);
  debug("| %d\t |\n", seg->dpl);
}
