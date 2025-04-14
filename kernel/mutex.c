#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mutex.h"

static int debug_enabled = 0;

void enable_mutex_debug(int on) {
    debug_enabled = on;
}

int is_mutex_debug_enabled(void) {
    return debug_enabled;
}

int
mutexalloc(struct file **f)
{
  struct file *fp;
  struct mutex *m;

  fp = filealloc();
  if(fp == 0) {
    if(is_mutex_debug_enabled()) printf("mutexalloc: filealloc failed\n"); // debug
    return -1;
  }
  
  if(is_mutex_debug_enabled()) printf("kalloc\n"); // debug
  
  m = kalloc();
  if(m == 0){
    if(is_mutex_debug_enabled()) printf("mutexalloc: kalloc failed\n"); // debug
    fileclose(fp);
    return -1;
  }

  if(is_mutex_debug_enabled()) printf("mutexalloc: created mutex\n"); // debug

  initsleeplock(&m->sl, "mutex sleep");
  initlock(&m->lk, "mutex internal");
  m->owner = 0;
  m->ref = 1;

  fp->type = FD_MUTEX;
  fp->readable = 0;
  fp->writable = 0;
  fp->mutex = m;
  fp->ref = 1;

  *f = fp;
  return 0; 
}

int
mutexunlock(struct file *f)
{
  if(is_mutex_debug_enabled()) printf("mutexunlock: called\n"); // debug

  if(f == 0 || f->type != FD_MUTEX) return -1;

  struct mutex *m = f->mutex;
  if (m == 0) return -1;
  
  acquire(&m->lk);
  if(m->owner != myproc()->pid){
    release(&m->lk);
    return -1;
  }
  m->owner = 0;
  release(&m->lk);
  releasesleep(&m->sl);
  return 0;
}

void
mutexclose(struct mutex *m) {
  if(is_mutex_debug_enabled()) printf("mutexclose\n"); // debug

  if (m == 0) return;
  
  acquire(&m->lk);
  if (m->owner != 0) {
      release(&m->lk);
      exit(1);
  }
  release(&m->lk);
  
  if(is_mutex_debug_enabled()) printf("kfree: freeing\n"); // debug
  kfree(m);
}
