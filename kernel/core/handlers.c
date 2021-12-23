#include <handlers.h>
#include <debug.h>
#include <types.h>
#include <intr.h>
#include <segment.h>

void print_isr() {
  asm volatile("leave");
  asm volatile("pusha");
  asm volatile("mov %esp, %eax");
  asm volatile("call print_handler");
  asm volatile("popa");
  asm volatile("iret");
}

void __regparm__(1) print_handler(int_ctx_t *ctx) {
  debug("Print syscall: %s\n", ctx->gpr.esi);
}

void set_handler(int index, int seg_idx, int dpl, void *handler) {
  idt_reg_t idtr;
  get_idtr(idtr);
  debug("IDT addresss: 0x%8x\n", (void *)idtr.desc);
  int_desc_t *id = &idtr.desc[index];
  int_desc(&idtr.desc[index], gdt_krn_seg_sel(seg_idx), (offset_t)handler);
  id->dpl = dpl;
}
