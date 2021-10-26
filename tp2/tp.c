/* GPLv4 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <intr.h>
#include <segmem.h>

extern info_t *info;

__attribute__((naked)) void bp_handler() {
  uint32_t eip;
  asm volatile("mov (%%esp), %0;" : "=r"(eip)::);
  asm volatile("pusha;" :::);
  printf("Bonjour, c'est Jean-Jacques Bourdin et le #BP est à 0x%8x\n", eip);
  asm volatile(
      "popa;"
      "iret;" ::
          :);
}

void bp_trigger() {
  printf("Bonjour, c'est Nicolas Sarkozy\n");
  asm volatile("int3" :::);
  printf("Je vais vous lire le temps de tempêtes, pour Audible\n");
}

void tp() {
  idt_reg_t idtr;
  get_idtr(idtr);

  // Get BP interruption "int3" => 3rd entry in IDT
  int_desc_t *int_bp = &idtr.desc[3];

  printf(
      "Inter \n"
      "type: 0x%8x\n"
      "priviledge: %d\n",
      int_bp->type, int_bp->dpl);

  int_desc(int_bp, gdt_krn_seg_sel(1), (offset_t)bp_handler);

  bp_trigger();

  printf("IDTR: 0x%8x\n", idtr.desc);
}
