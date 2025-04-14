#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/procinfo.h"

#define INITIAL_BUF_SIZE 4

void spawn_extra_processes(int num) {
  for (int i = 0; i < num; i++) {
    int pid = fork();
    if (pid == -1) {
      fprintf(2, "fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      while(1)
        sleep(50);
      break;
    }
  }
}

void test_buffer_too_small() {
  spawn_extra_processes(7);

  struct procinfo buf[INITIAL_BUF_SIZE];
  int nprocs = ps_listinfo(buf, INITIAL_BUF_SIZE);

  if (nprocs == -1) {
    printf("Passed buffer too small test\n");
  } else if (nprocs >= 0) {
    printf("Failed buffer too small test, buffer was sufficient unexpectedly, got %d processes\n", nprocs);
  } else {
    printf("Failed buffer too small test, unexpected error: %d\n", nprocs);
  }
}

void test_invalid_address() {
  int nprocs = ps_listinfo((struct procinfo*)0x1, 10);
  if(nprocs == -2)
    printf("Passed invalid address test\n");
  else
    printf("Failed invalid address test: %d\n", nprocs);
}

int main() {
  test_invalid_address();
  test_buffer_too_small();

  exit(0);
}
