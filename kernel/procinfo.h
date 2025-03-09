#define PROCNAME_LEN 16

enum n_procstate { n_UNUSED, n_USED, n_SLEEPING, n_RUNNABLE, n_RUNNING, n_ZOMBIE };

struct procinfo {
  int pid;
  char name[PROCNAME_LEN];
  enum n_procstate state;
  int ppid;
  char pname[PROCNAME_LEN];
};
