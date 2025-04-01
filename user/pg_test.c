#include "kernel/types.h"
#include "user.h"

#define PAGE_SIZE 4096
#define BIG_SIZE (PAGE_SIZE * 4)

int global_var;

int main() {
  printf("===== Initial State =====\n");
  pginfo(0, 0, 0);
  
  int *heap_array = malloc(BIG_SIZE);
  if (!heap_array) {
      fprintf(2, "malloc failed\n");
      exit(1);
  }
  memset(heap_array, 0, BIG_SIZE);

  int stack_array[PAGE_SIZE / sizeof(int)];

  printf("\n===== After Allocation =====\n");
  pginfo(0, 0, 0);

  printf("\n===== Clear A/D Flags =====\n");
  pgclear(0, 0, 3);
  pginfo(0, 0, 0);

  int tmp = global_var;
  tmp = stack_array[0];
  tmp = heap_array[0];
  tmp = heap_array[PAGE_SIZE / sizeof(int)];

  printf("\n===== After Read =====\n");
  printf("tmp=%d", tmp);
  pginfo(0, 0, 0);

  stack_array[0] = 42;
  heap_array[0] = 42;
  heap_array[PAGE_SIZE / sizeof(int)] = 42;

  printf("\n===== After Write =====\n");
  pginfo(0, 0, 0);

  free(heap_array);
  printf("\n===== After Free =====\n");
  pginfo(0, 0, 0);

  exit(0);
}
