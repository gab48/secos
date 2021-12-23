#ifndef __TASKS_H__
#define __TASKS_H__
#include <segmem.h>
#include <intr.h>
#include <page.h>

typedef struct task_descriptor {
  int        id;
  void     *fct;
  uint32_t  esp;
  uint32_t  ebp;
  uint32_t  eip;
  pde32_t  *pgd;
} __attribute__((packed)) task_desc_t;

typedef struct task_descriptor_table {
  task_desc_t *task;
  uint32_t size;
} __attribute__((packed)) tdt_t;

void tasks_init(tss_t *tss);
void task_scheduler(int_ctx_t ctx);
void load_task(int id, pde32_t *pgd, uint32_t esp, uint32_t ebp, void *fct); 

#endif
