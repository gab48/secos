#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include <segmem.h>

#define NUMBER_OF_SEGMENT 6
#define SEG_CODE_R0 1
#define SEG_DATA_R0 2
#define SEG_CODE_R3 3
#define SEG_DATA_R3 4
#define SEG_TSS 5

void init_seg(seg_desc_t *seg);
void set_null_seg(seg_desc_t *seg);
void set_seg_r0(seg_desc_t *seg);
void set_seg_r3(seg_desc_t *seg);
void print_seg(seg_desc_t *seg);

#endif
