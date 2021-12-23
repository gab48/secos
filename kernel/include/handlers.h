#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define IDT_IRQ0_IDX 32
#define IDT_PRINT_HANDLER_IDX 0x80

void print_isr();
void set_handler(int index, int seg_idx, int dpl, void *handler);

#endif
