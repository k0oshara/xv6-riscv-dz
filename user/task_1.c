#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int pid, status;

  pid = fork();
  if (pid == -1) {
    fprintf(2, "fork error\n");
    exit(1);
  }

  if (pid == 0) {
    sleep(10);
    exit(1);
  } 
  else if (pid > 0) {
    printf("parent pid: %d\nchild pid: %d\n", getpid(), pid);
    
    if (argc > 1 && strcmp(argv[1], "kill") == 0) {
      if (kill(pid) == -1) {
        fprintf(2, "kill error: failed to send signal to pid=%d\n", pid);
        exit(1);
      }
    }
     
    int wpid = wait(&status);
    printf("child, wpid=%d, exited with status=%d\n", wpid, status);
    exit(0);
  }  
}
