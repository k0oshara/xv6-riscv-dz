#include "kernel/types.h"
#include "user.h"

void
print_unsync(int argc, char *argv[])
{
  int i, j;
  char tmp[2];
  for(i = 1; i < argc; i++){
    for(j = 0; argv[i][j] != '\0'; j++){
      tmp[0] = argv[i][j];
      tmp[1] = '\0';
      printf("pid %d: arg %d, char '%s'\n", getpid(), i, tmp);
    }
  }
}

void
print_sync(int argc, char *argv[], int mfd)
{
  int i, j;
  char tmp[2];
  for(i = 1; i < argc; i++){
    for(j = 0; argv[i][j] != '\0'; j++){
      if(mutex_lock(mfd) < 0){
        fprintf(2, "mutex_lock failed\n");
        exit(1);
      }
      tmp[0] = argv[i][j];
      tmp[1] = '\0';
      printf("pid %d: arg %d, char '%s'\n", getpid(), i, tmp);
      if(mutex_unlock(mfd) < 0){
        fprintf(2, "mutex_unlock failed\n");
        exit(1);
      }
    }
  }
}

int
main(int argc, char *argv[]) {
  if (argc < 2) {
      fprintf(2, "Usage: %s arg1\n", argv[0]);
      exit(1);
  }

  printf("=== Without mutex ===\n");
  for (int i = 0; i < 2; i++) {
      if (fork() == 0) {
          print_unsync(argc, argv);
          exit(0);
      }
  }
  for (int i = 0; i < 2; i++) wait(0);

  sleep(10);

  printf("\n=== With mutex ===\n");
  int mfd = mutex();
  if (mfd < 0) exit(1);
  
  for (int i = 0; i < 2; i++) {
      if (fork() == 0) {
          print_sync(argc, argv, mfd);
          exit(0);
      }
  }
  for (int i = 0; i < 2; i++) wait(0);

  close(mfd);
  exit(0);
}
