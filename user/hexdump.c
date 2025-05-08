#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define LINE_SIZE 48

const char hex_chars[] = "0123456789abcdef";

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "Usage: hexdump <file> <bytes>\n");
    exit(1);
  }

  int n = atoi(argv[2]);
  if (n <= 0) {
    fprintf(2, "hexdump: invalid size (must be positive)\n");
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(2, "hexdump: cannot open %s\n", argv[1]);
    exit(1);
  }

  char* buf = malloc(n);
  if(!buf) {
    fprintf(2, "hexdump: memory error\n");
    close(fd);
    exit(1);
  }

  int bytes_read = read(fd, buf, n);
  if (bytes_read < 0) {
    fprintf(2, "hexdump: read error\n");
    free(buf);
    close(fd);
    exit(1);
  }

  char line[LINE_SIZE];
  int line_pos = 0;

  for (int i = 0; i < bytes_read; i++) {
    uint8 byte = buf[i] & 0xFF;
  
    line[line_pos++] = hex_chars[(byte >> 4) & 0xF];
    line[line_pos++] = hex_chars[byte & 0xF];
    line[line_pos++] = ' ';

    if ((i + 1) % 16 == 0) {
      line[line_pos - 1] = '\n';
      write(1, line, line_pos);
      line_pos = 0;
    }
  }

  if (line_pos > 0) {
    line[line_pos - 1] = '\n';
    write(1, line, line_pos);
  }

  close(fd);
  exit(0);
}
