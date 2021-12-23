#ifndef __GDT_H__
#define __GDT_H__

#include <types.h>
#include <segmem.h>

void gdt_init();
void gdt_add_segment(uint16_t type, uint16_t dpl);
void gdt_add_tss(tss_t *tss);
void gdt_print();

#endif
