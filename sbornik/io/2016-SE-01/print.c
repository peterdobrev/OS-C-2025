#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void print_number(int fd, uint8_t cur) {
  char num[4];
  snprintf(num, 4, "%d", cur);
  write(fd, num, strlen(num));
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    errx(1, "Pass a file!");
  }

  int fd = open(argv[1], O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open file");
  }

  uint8_t cur;
  int bytes;
  while((bytes = read(fd, &cur, sizeof(cur)))) {
    print_number(1, cur);
    write(1, "\n", 1);
  }

  if(bytes < 0) {
    close(fd);
    err(1, "Couldn't read");
  }

  close(fd);

  return 0;
}
