#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int writeall(int fd, const void *buf, int count) {
  int bytes_written = 0;
  const char *ptr = buf;
  while (bytes_written < count) {
    int n = write (fd, ptr + bytes_written, count - bytes_written);
    if (n == -1) {
      return n;
    }
   bytes_written += n; 
  }
  return bytes_written;
}

int safe_close(int fd) {
  if (close(fd) == -1) {
    fprintf(2, "close error\n");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int pipefd[2];
  int pid;

  if (pipe(pipefd) == -1) {
    fprintf(2, "pipe error\n");
    exit(1);
  }

  pid = fork();
  if (pid == -1) {
    fprintf(2, "fork error\n");
    safe_close(pipefd[0]);
    safe_close(pipefd[1]);
    exit(1);
  }

  if (pid == 0) { 
    if (safe_close(pipefd[1]) == -1)
      exit(1);
    
    if (safe_close(0) == -1)
      exit(1);
    if (dup(pipefd[0]) == -1)
      exit(1);
    if (safe_close(pipefd[0]) == -1)
      exit(1);
    
    char *wc_argv[] = {"/wc", 0};
    if (exec("/wc", wc_argv) == -1) {
      fprintf(2, "exec error\n");
      exit(1);
    }
  } else if (pid > 0) {
    if (safe_close(pipefd[0]) == -1)
      exit(1);
    
    for (int i = 1; i < argc; i++) {
      int len = strlen(argv[i]);
      if (writeall(pipefd[1], argv[i], len) != len || 
          writeall(pipefd[1], "\n", 1) != 1) {
        fprintf(2, "write error\n");
        safe_close(pipefd[1]);
        exit(1);
      }
    }
    if (safe_close(pipefd[1]) == -1)
      exit(1);
    if (wait((int*)0) == -1) {
      fprintf(2, "wait error\n");
      exit(1);
    }
    exit(0);
  }
}
