#include "types.h"
#include "fs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

static struct spinlock urandom_lock;
static uint urandom_seed = 1;
static struct spinlock nullstat_lock;
static uint64 nullstat_bytes = 0;

#define ZERO_BUF_SIZE 4096
static char zero_buf[ZERO_BUF_SIZE] = {0};
#define min(a, b) ((a) < (b) ? (a) : (b))

static int zero_read(uint64 dst, int n) {
  int total = 0;
  while (total < n) {
    int ch = min(n - total, ZERO_BUF_SIZE);
    if (either_copyout(1, dst + total, zero_buf, ch) < 0) return -1;
    total += ch;
  }
  return total;
}

static int urandom_read(uint64 dst, int n) {
  acquire(&urandom_lock);
  for (int i = 0; i < n; i++) {
    urandom_seed = (1664525 * urandom_seed + 1013904223) % 4294967296;
    char byte = (urandom_seed >> 24) & 0xFF;
    if (either_copyout(1, dst + i, &byte, 1) < 0) {
      release(&urandom_lock);
      return -1;
    }
  }
  release(&urandom_lock);
  return n;
}

static int urandom_write(uint64 src, int n) {
  if (n != sizeof(uint)) return -1;
  uint new_seed;
  if (either_copyin(&new_seed, 1, src, sizeof(new_seed))) return -1;
  acquire(&urandom_lock);
  urandom_seed = new_seed;
  release(&urandom_lock);
  return sizeof(new_seed);
}

static int nullstat_read(uint64 dst, int n) {
  if (n != sizeof(nullstat_bytes)) return -1;
  acquire(&nullstat_lock);
  int ret = either_copyout(1, dst, &nullstat_bytes, sizeof(nullstat_bytes));
  release(&nullstat_lock);
  return (ret < 0) ? -1 : sizeof(nullstat_bytes);
}

static int nullstat_write(int n) {
  acquire(&nullstat_lock);
  nullstat_bytes += n;
  release(&nullstat_lock);
  return n;
}

int pseudo_read(short minor, int user_dst, uint64 dst, int n) {
  switch (minor) {
    case NULL_MINOR:     return 0;
    case ZERO_MINOR:     return zero_read(dst, n); 
    case URANDOM_MINOR:  return urandom_read(dst, n);
    case NULLSTAT_MINOR: return nullstat_read(dst, n);
    default:             return -1;
  }
}

int pseudo_write(short minor, int user_src, uint64 src, int n) {
  switch (minor) {
    case NULL_MINOR:     return n;
    case ZERO_MINOR:     return -1;
    case URANDOM_MINOR:  return urandom_write(src, n);
    case NULLSTAT_MINOR: return nullstat_write(n);
    default:             return -1;
  }
}

void pseudo_dev_init(void) {
  initlock(&urandom_lock, "urandom");
  initlock(&nullstat_lock, "nullstat");
  devsw[PSEUDO_MAJOR].read = pseudo_read;
  devsw[PSEUDO_MAJOR].write = pseudo_write;
}
