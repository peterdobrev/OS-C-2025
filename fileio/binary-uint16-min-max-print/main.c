#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>

int n_digits(uint16_t num) {
  if(num == 0) return 1;
  int res = 0;
  while(num) {
    res++;
    num/=10;
  }
  return res;
}

void num_to_text(uint16_t num, char* buf) {
  buf[n_digits(num)] = '\0';
  for(int i = n_digits(num) - 1; i >= 0; i--) {
    buf[i] = '0' + (num % 10);
    num /= 10;
  }
}

void print_number(int fd, uint16_t num) {
  char num_text[6];
  num_to_text(num, num_text);
  write(fd, num_text, n_digits(num));
}

void print(int fd) {
  uint16_t cur;
  int bytes;
  while((bytes = read(fd, &cur, sizeof(cur))) > 0) {
    if(bytes != sizeof(cur)) {
      close(fd);
      err(1, "Error while reading from file: ");
    }
    write(1, "\n", 1);
    print_number(1, cur);
  }

  if(bytes < 0) {
    close(fd);
    err(1, "Error when reading from file: ");
  }
}

void find(int fd, int isMax) {
  uint16_t cur, tofind;
  int first = 1;
  int bytes;
  while((bytes = read(fd,&cur ,sizeof(cur))) > 0) {
    if(bytes != sizeof(cur)) {
      close(fd);
      err(1, "Error while reading from file: ");
    }

    if(first) {
      first = 0;
      tofind = cur;
    } else {
      if((isMax && cur > tofind)
        || (!isMax && cur < tofind)) {
        tofind = cur;
      }
    }
  }
  if(bytes < 0) {
    close(fd);
    err(1, "Error when reading from file: ");
  }

  print_number(1, tofind);
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    errx(1, "2 arguments are needed");
  }

  int fd = open(argv[2], O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open file %s.", argv[2]);
  }

  if(strcmp(argv[1], "--min") == 0) {
    find(fd, 0);
  } else if(strcmp(argv[1], "--max") == 0) {
    find(fd, 1);
  } else if(strcmp(argv[1], "--print") == 0) {
    print(fd); 
  } else {
    errx(1, "Invalid argument: %s", argv[1]);
  }

  close(fd);
}
