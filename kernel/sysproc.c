#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

#define MASK_A (1 << 1)
#define MASK_D 1

static int 
check_flags(pte_t pte, int flags) 
{
  if (flags == 0) return 1;
  int mask = (flags & MASK_A ? PTE_D : 0) | (flags & MASK_D ? PTE_A : 0);
  return (mask != 0) && ((pte & mask) == mask);
}

static void 
print_pagetable(pagetable_t pagetable, int lvl, int flags) 
{
  if (lvl < 0) return;
  for (int i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];

    if (!(pte & PTE_V)) continue;
    
    uint64 pa = PTE2PA(pte);
    if (check_flags(pte, flags)) {
      char flags_str[] = {
          (pte & PTE_R) ? 'R' : '_',
          (pte & PTE_W) ? 'W' : '_',
          (pte & PTE_X) ? 'X' : '_',
          (pte & PTE_U) ? 'U' : '_',
          (pte & PTE_G) ? 'G' : '_',
          (pte & PTE_A) ? 'A' : '_',
          (pte & PTE_D) ? 'D' : '_',
          '\0'
      };
      int indent = (2 - lvl) * 9;
      char indent_str[indent + 1];
      memset(indent_str, '.', indent);
      indent_str[indent] = '\0';
      printf("%s 0x%x -> 0x%lx %s\n", indent_str, i, pa, flags_str);
    }
    if ((pte & (PTE_R | PTE_W | PTE_X)) == 0) {
      print_pagetable((pagetable_t)pa, lvl - 1, flags);
    }
  }
}

static int 
validate_buffer(uint64 start, uint64 len, struct proc *p) 
{
  uint64 end = start + len;
  if (start >= p->sz || end < start || end > p->sz) return -1;
  for (uint64 va = start; va < end; va += PGSIZE) {
      pte_t *pte = walk(p->pagetable, va, 0);
      if (!pte || !(*pte & PTE_V) || !(*pte & PTE_U)) return -1;
  }
  return 0;
}

int
sys_pginfo(void)
{
  uint64 buf;
  int len, flags;
  struct proc *p = myproc();
  
  argaddr(0, &buf);
  argint(1, &len);
  argint(2, &flags);
  
  if (flags & ~(MASK_A | MASK_D))  return -1;
  if ((buf != 0 || len != 0) && validate_buffer(buf, len, p) < 0) return -1;

  printf("PAGETABLE 0x%lx\n", (uint64)p->pagetable - KERNBASE);

  if (buf == 0 && len == 0) {
    print_pagetable(p->pagetable, 2, flags);
  } 
  else {
    uint64 start = PGROUNDDOWN(buf);
    uint64 end = PGROUNDUP(buf + len);
    
    for (uint64 va = start; va < end; va += PGSIZE) {
      pagetable_t pt = p->pagetable;
      for (int level = 2; level >= 0; level--) {
        int shift = 12 + 9 * level;
        uint64 idx = (va >> shift) & 0x1FF;
        pte_t *pte = &pt[idx];

        if (!(*pte & PTE_V)) break;
        if (check_flags(*pte, flags)) {
          char flags_str[] = {
              (*pte & PTE_R) ? 'R' : '_',
              (*pte & PTE_W) ? 'W' : '_',
              (*pte & PTE_X) ? 'X' : '_',
              (*pte & PTE_U) ? 'U' : '_',
              (*pte & PTE_G) ? 'G' : '_',
              (*pte & PTE_A) ? 'A' : '_',
              (*pte & PTE_D) ? 'D' : '_',
              '\0'
          };
          int indent = (2 - level) * 9;
          char indent_str[indent + 1];
          memset(indent_str, '.', indent);
          indent_str[indent] = '\0';
          printf("%s 0x%lx -> 0x%lx %s\n", indent_str, idx, PTE2PA(*pte), flags_str);
        }
      
        if (level == 0) break;
        if ((*pte & (PTE_R | PTE_W | PTE_X)) != 0) break;
        pt = (pagetable_t)PTE2PA(*pte);
      }
    }
  }
  return 0;
}

static void 
clear_pagetable(pagetable_t pagetable, int lvl, int flags) 
{
  if (lvl < 0) return;
  for (int i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];
    if (!(pte & PTE_V)) continue;

    if (pte & PTE_U) {
      pte_t new_pte = pte;
      if (flags & MASK_D) new_pte &= ~PTE_D;
      if (flags & MASK_A) new_pte &= ~PTE_A;
      pagetable[i] = new_pte;
    }

    if ((pte & (PTE_R | PTE_W | PTE_X)) == 0) {
      clear_pagetable((pagetable_t)PTE2PA(pte), lvl - 1, flags);
    }
  }
}

int 
sys_pgclear(void) 
{
  uint64 buf;
  int len, flags;
  struct proc *p = myproc();

  argaddr(0, &buf);
  argint(1, &len);
  argint(2, &flags);
  
  if (flags & ~(MASK_A | MASK_D)) return -1;
  if (buf == 0 && len != 0) return -1;
  if ((buf != 0 || len != 0) && validate_buffer(buf, len, p) < 0) return -1;

  if (buf == 0 && len == 0) {
    clear_pagetable(p->pagetable, 2, flags);
  } 
  else {
    uint64 start = PGROUNDDOWN(buf);
    uint64 end = PGROUNDUP(buf + len);
    for (uint64 va = start; va < end; va += PGSIZE) {
      pte_t *pte = walk(p->pagetable, va, 0);
      if (pte && (*pte & PTE_V) && (*pte & PTE_U)) {
        if (flags & MASK_D) *pte &= ~PTE_D;
        if (flags & MASK_A) *pte &= ~PTE_A;
      }
    }
  }
  
  sfence_vma();
  return 0;
}
