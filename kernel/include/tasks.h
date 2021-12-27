#ifndef __TASKS_H__
#define __TASKS_H__
#include <segmem.h>
#include <intr.h>
#include <page.h>

typedef struct task_descriptor {
  int        id;
  uint32_t  eax;
  uint32_t  ebx;
  uint32_t  ecx;
  uint32_t  edx;
  uint32_t  esi;
  uint32_t  edi;
  uint32_t  eip;
  uint32_t  esp;
  uint32_t  ebp;
  uint32_t  junk;
  uint32_t  stack_r0;
  uint32_t  stack_r3;
  pde32_t  *pgd;
} __attribute__((packed)) task_desc_t;

typedef struct task_descriptor_table {
  task_desc_t *task;
  uint32_t size;
} __attribute__((packed)) tdt_t;

void tasks_init(tss_t *tss);
void task_scheduler();
void load_task(int id, pde32_t *pgd, void *fct, uint32_t stack0, uint32_t stack3); 

#endif
