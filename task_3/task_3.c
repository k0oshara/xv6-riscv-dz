#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUF_SIZE 10240

ssize_t writeall(int fd, const void *buf, size_t count) {
  size_t bytes_written = 0;
  const char *ptr = buf;
  while (bytes_written < count) {
    ssize_t n = write (fd, ptr + bytes_written, count - bytes_written);
    if (n == -1) {
      if (errno == EINTR)
        continue;
      return n;
    }
   bytes_written += n; 
  }
  return bytes_written;
}

int safe_close(int fd) {
  if (close(fd) == -1) {
    perror("close");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int pipefd[2];
  pid_t pid;

  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  pid = fork();
  if (pid == -1) {
    perror("fork");
    safe_close(pipefd[0]);
    safe_close(pipefd[1]);
    exit(EXIT_FAILURE);
  }

  if (pid == 0) { 
    if (safe_close(pipefd[1]) == -1)
      exit(EXIT_FAILURE);
    
    char buffer[BUF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(pipefd[0], buffer, BUF_SIZE)) > 0) {
      ssize_t bytes_written = 0;
      while (bytes_written < bytes_read) {
        ssize_t n = write(STDOUT_FILENO, buffer + bytes_written, bytes_read - bytes_written);
        if (n == -1) {
          if (errno == EINTR)
            continue;
          perror("write to stdout");
          safe_close(pipefd[0]);
          exit(EXIT_FAILURE);
        }
        bytes_written += n;
      } 
    }
    if (bytes_read == -1) {
      perror("read");
      safe_close(pipefd[0]);
      exit(EXIT_FAILURE);
    }
    if (safe_close(pipefd[0]) == -1)
      exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
  } 
  else if (pid > 0) {
    if (safe_close(pipefd[0]) == -1)
      exit(EXIT_FAILURE);
    
    for (int i = 1; i < argc; i++) {
      size_t len = strlen(argv[i]);
      if (writeall(pipefd[1], argv[i], len) != len || 
          writeall(pipefd[1], "\n", 1) != 1) {
        perror("write");
        safe_close(pipefd[1]);
        exit(EXIT_FAILURE);
      }
    }

    if(safe_close(pipefd[1]) == -1)
      exit(EXIT_FAILURE);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status))
      return WEXITSTATUS(status);
    else {
      fprintf(stderr, "child terminated abnormally\n");
      exit(EXIT_FAILURE);
    }
  }
}
