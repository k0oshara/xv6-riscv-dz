#include <kernel/types.h>
#include <user/user.h>

#define BUF_SIZE 100

int is_valid_number(char* s) {
  if (*s == '\0')
    return 0;
  for (int i = 0; s[i] != '\0'; i++) {
    if (s[i] < '0' || s[i] > '9') {
      return 0;
    }
  }
  return 1;
}

int main() {
  char buf[BUF_SIZE];
  int i, cc;
  char c;

  for(i = 0; i + 1 < BUF_SIZE;) {
    cc = read(0, &c, 1);
    if (cc < 0) {
      fprintf(2, "error: reading\n");
      exit(1);
    }
    if (cc == 0) break;
    if (c == '\n' || c == '\r') {
      buf[i] = '\0';
      break;
    }
    buf[i++] = c;
  }
  
  printf("|%s|\n", buf);
  
  if (i == BUF_SIZE - 1 && buf[i - 1] != '\0') {
    fprintf(2, "error: buffer overflow\n");
    exit(1);
  }

  if (buf[i] != '\0') 
    buf[i] = '\0';

  if (i == 0) {
    fprintf(2, "error: empty string\n");
    exit(1);
  }

  int space_ind = -1;
  for(int j = 0; j < i; j++) {
    if (buf[j] == ' ') {
      space_ind = j;
      break;
    }
  }
  if (space_ind == -1) {
    fprintf(2, "error: missing space between arguments\n");
    exit(1);
  }

  buf[space_ind] = '\0';

  if (buf[0] == '\0' || buf[space_ind + 1] == '\0') {
    fprintf(2, "error: one of the numbers is empty\n");
    exit(1);
  }

  char* sec =  &buf[space_ind + 1];
  for (int j = 0; sec[j] != '\0'; j++) {
    if (sec[j] == ' ') {
      fprintf(2, "error: more than two arguments\n");
      exit(1);
    }
  }

  if (!is_valid_number(buf)) {
    fprintf(2, "error: the first number is incorrect\n");
    exit(1);
  }

  if (!is_valid_number(&buf[space_ind + 1])) {
    fprintf(2, "error: the second number is incorrect\n");
    exit(1);
  }

  int a = atoi(buf);
  int b = atoi(&buf[space_ind + 1]);

  printf("%d\n", a + b);
  exit(0);
}
