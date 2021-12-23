#include <segment.h>
#include <default.h>
#include <segmem.h>
#include <tasks.h>
#include <debug.h>
#include <intr.h>
#include <pic.h>
#include <asm.h>
#include <cr.h>
#include <io.h>

#define TASKS_NUMBER 2

tdt_t  tdt;
tss_t *tss_addr;
task_desc_t *cur_task = NULL;

void tasks_init(tss_t *tss) {
  tdt.size = 0;
  tss_addr=tss;
}

void task_scheduler(int_ctx_t ctx) {
  outb(PIC_EOI, PIC1);
  debug("Je schedule ses grands morts\n");
  debug("esp = 0x%8x\n", ctx.esp);
  
  if (cur_task == NULL) { // Initialisation
    void *usrlnd_ptr = tdt.task[1].fct;
    tss_addr->s0.esp = get_esp();
    set_cr3(tdt.task[1].pgd);
    
    asm volatile("movl %0, %%esp"::"r"(0x40ffff));
    asm volatile("movl %esp, %eax");
    asm volatile("push %0" ::"i"(gdt_seg_sel(SEG_DATA_R3, RING_3)));
    asm volatile("push %eax");
    asm volatile("pushf");
    asm volatile("push %0" ::"i"(gdt_seg_sel(SEG_CODE_R3, RING_3)));
    asm volatile("push %%ebx" ::"b"(usrlnd_ptr));
    //asm volatile("iret");
  } 
  
  force_interrupts_on();
}  

void load_task(int id, pde32_t *pgd, uint32_t esp, uint32_t ebp, void *fct) {
  tdt.task[id].id  =  id;
  tdt.task[id].pgd = pgd;
  tdt.task[id].esp = esp;
  tdt.task[id].ebp = ebp;
  tdt.task[id].fct = fct;
  tdt.size++;
}
