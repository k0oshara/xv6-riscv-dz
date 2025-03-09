#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

#define INITIAL_BUF_SIZE 8

const char* state_to_str(enum n_procstate state) {
  switch(state){
    case n_UNUSED:   return "UNUSED";
    case n_USED:     return "USED";
    case n_SLEEPING: return "SLEEPING";
    case n_RUNNABLE: return "RUNNABLE";
    case n_RUNNING:  return "RUNNING";
    case n_ZOMBIE:   return "ZOMBIE";
    default:         return "UNKNOWN";
  }
}

int main() {
  int bufsize = INITIAL_BUF_SIZE;
  struct procinfo *buf;
  int nprocs;

  while(1) {
    buf = malloc(sizeof(struct procinfo) * bufsize);
    if (!buf) {
      fprintf(2, "Memory allocation failed.\n");
      exit(1);
    }

    nprocs = ps_listinfo(buf, bufsize);

    if (nprocs == -1) {
      free(buf);
      bufsize *= 2;
      continue;
    } else if (nprocs == -2) {
      fprintf(2, "Error: invalid address or copyout failure (-2)\n");
      free(buf);
      exit(1);
    }

    printf("PID\tNAME\tSTATE\t  PPID\tPNAME\n");
    for(int i = 0; i < nprocs; i++) {
      printf("%d\t%s\t%s  %d\t%s\n",
        buf[i].pid,
        buf[i].name,
        state_to_str(buf[i].state),
        buf[i].ppid,
        buf[i].pname
      );
    }

    free(buf);
    break;
  }
  exit(0);
}
