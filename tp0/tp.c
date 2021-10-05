/* GPLv2 (c) Airbus */
#include <debug.h>
#include <grub_mbi.h>
#include <info.h>

extern info_t *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

// Q3 -> Pas d'erreurs (seg fault) mais impossible d'écrire
// Q4 -> Pas d'erreurs mais renvoie 0 

void tp() {
  debug("kernel mem [0x%x - 0x%x]\n", &__kernel_start__, &__kernel_end__);
  debug("MBI flags 0x%x\n", info->mbi->flags);
  multiboot_memory_map_t * start=(multiboot_memory_map_t *)info->mbi->mmap_addr;
  multiboot_memory_map_t * end=(multiboot_memory_map_t *)(info->mbi->mmap_addr + info->mbi->mmap_length);

  while (start < end) {

    debug("[0x%8x]-", start->addr);
    debug("[0x%8x]: ", start->addr+start->len-1);
    debug(" %d\n", start->type);

    
    start++;
  }
  int * test = (int *)0xffffffffff;
  *test=42;
  debug("Valeur de test: %d\n", *test);
        
}
