#include "kernel/types.h"
#include "user.h"

#define MASK_A (1 << 1)
#define MASK_D 1

#define PAGE_SIZE 4096
#define BIG_SIZE (PAGE_SIZE * 4)

int global_var;

int main()
{
  printf("===== Initial State =====\n");
  pginfo(0, 0, 0);

  printf("\n===== Initial State - Only Modified Pages (D) =====\n");
  pginfo(0, 0, MASK_D);

  printf("\n===== Initial State - Only Accessed Pages (A) =====\n");
  pginfo(0, 0, MASK_A);

  int *heap_array = malloc(BIG_SIZE);
  if (!heap_array)
  {
    fprintf(2, "malloc failed\n");
    exit(1);
  }
  memset(heap_array, 0, BIG_SIZE);

  int stack_array[PAGE_SIZE / sizeof(int)];

  printf("\n===== After Allocation =====\n");
  pginfo(0, 0, 0);

  printf("\n===== Clear A/D Flags =====\n");
  pgclear(0, 0, MASK_A | MASK_D);
  pginfo(0, 0, 0);

  printf("\n===== After Clearing - Only Modified Pages (D) =====\n");
  pginfo(0, 0, MASK_D);

  printf("\n===== After Clearing - Only Accessed Pages (A) =====\n");
  pginfo(0, 0, MASK_A);

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

  printf("\n===== After Write - Only Modified Pages (D) =====\n");
  pginfo(0, 0, MASK_D);

  free(heap_array);
  printf("\n===== After Free =====\n");
  pginfo(0, 0, 0);

  exit(0);
}
