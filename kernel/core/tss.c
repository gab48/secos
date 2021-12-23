#include <tss.h>
#include <string.h>
#include <segmem.h>
#include <segment.h>
#include <default.h>

void init_tss(tss_t *tss) {
  memset(tss, 0, sizeof(*tss));
  tss->s0.esp = get_ebp();
  tss->s0.ss = gdt_seg_sel(SEG_DATA_R0, RING_0);
  set_tr(gdt_seg_sel(SEG_TSS, RING_0));
}

