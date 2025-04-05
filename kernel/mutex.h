struct mutex {
  struct sleeplock sl;
  struct spinlock lk;
  int owner;
  int ref;
};

int
mutexalloc(struct file **f);

int
mutexunlock(struct file *f);

void
mutexclose(struct mutex *m);
