#include <asm.h>
#include <cr.h>
#include <debug.h>
#include <default.h>
#include <intr.h>
#include <io.h>
#include <gpr.h>
#include <pic.h>
#include <segmem.h>
#include <segment.h>
#include <tasks.h>

#define NUMBER_OF_TASKS 2

tdt_t tdt;
tss_t *tss_addr;
task_desc_t *cur_task = NULL;

void tasks_init(tss_t *tss) {
  tdt.size = 0;
  tss_addr = tss;
}

void __set_ring3_segments() {
  set_ds(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_es(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_fs(gdt_seg_sel(SEG_DATA_R3, RING_3));
  set_gs(gdt_seg_sel(SEG_DATA_R3, RING_3));
}

__attribute__((naked)) void task_scheduler() {
  outb(PIC_EOI, PIC1);

  if (cur_task == NULL) {  // Initialisation
    cur_task = &tdt.task[1];
    __set_ring3_segments();
  } else {
    // Save curent task
    asm volatile("mov %%eax, %0" : "=r"(cur_task->eax));
    asm volatile("mov %%ebx, %0" : "=r"(cur_task->ebx));
    asm volatile("mov %%ecx, %0" : "=r"(cur_task->ecx));
    asm volatile("mov %%edx, %0" : "=r"(cur_task->edx));
    asm volatile("mov %%esi, %0" : "=r"(cur_task->esi));
    asm volatile("mov %%edi, %0" : "=r"(cur_task->edi));
    asm volatile("pop %0" : "=r"(cur_task->eip));
    asm volatile("pop %eax");
    asm volatile("pop %eax");
    asm volatile("pop %0" : "=r"(cur_task->esp3));
    asm volatile("pop %eax");
    asm volatile("mov %%esp, %0" : "=r"(cur_task->esp0));
    asm volatile("mov %%ebp, %0" : "=r"(cur_task->ebp));
    cur_task =
        &tdt.task[cur_task->id % NUMBER_OF_TASKS + 1];  // Switch user1 to user2

    debug("Switch to user%d\n", cur_task->id);
    __set_ring3_segments();
    asm volatile("mov %0, %%esp" ::"r"(cur_task->esp0));
    asm volatile("mov %0, %%ebp" ::"r"(cur_task->ebp));
    asm volatile("mov %0, %%eax" ::"r"(cur_task->eax));
    asm volatile("mov %0, %%ebx" ::"r"(cur_task->ebx));
    asm volatile("mov %0, %%ecx" ::"r"(cur_task->ecx));
    asm volatile("mov %0, %%edx" ::"r"(cur_task->edx));
    asm volatile("mov %0, %%esi" ::"r"(cur_task->esi));
    asm volatile("mov %0, %%edi" ::"r"(cur_task->edi));
  }

  tss_addr->s0.esp = cur_task->esp0;
  set_cr3(cur_task->pgd);
  asm volatile("push %0" ::"i"(gdt_seg_sel(SEG_DATA_R3, RING_3)));
  asm volatile("push %0" ::"r"(cur_task->esp3));

  // Set eflag IF
  asm volatile("pushf"); 
  asm volatile("pop %0":"=m"(cur_task->eflags));
  cur_task->eflags = cur_task->eflags | EFLAGS_IF;
  asm volatile("push %0"::"m"(cur_task->eflags));

  asm volatile("push %0" ::"i"(gdt_seg_sel(SEG_CODE_R3, RING_3)));
  asm volatile("push %%ebx" ::"b"((void *)cur_task->eip));
  asm volatile("iret");
}

void load_task(int id, pde32_t *pgd, void *fct, uint32_t stack0,
               uint32_t stack3) {
  tdt.task[id].id = id;
  tdt.task[id].pgd = pgd;
  tdt.task[id].eip = (uint32_t)fct;
  tdt.task[id].esp0 = stack0;
  tdt.task[id].esp3 = stack3;
  tdt.size++;
}
