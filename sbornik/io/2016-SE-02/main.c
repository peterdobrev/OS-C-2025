#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

int write_interval(uint32_t x, uint32_t y, int fd_from, int fd_to) {
  int pos = lseek(fd_from, x, SEEK_SET);
  if(pos < 0) {
    // err
    return 1;
  }

  uint32_t cur;
  int bytes;
  while(y-- && (bytes = read(fd_from, &cur, sizeof(cur))) > 0) {
    if(bytes != sizeof(cur)) {
      return 1;
    }

    int written = write(fd_to, &cur, sizeof(cur));
    if(written < 0) {
      return 1;
    }
  }

  if(bytes < 0) {
    return 1;
  }

  if(y > 0) {
    // edge case handling
    return 0;
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 4) {
    errx(1, "Pass 3 files!");  
  }

  int fd1 = open(argv[1], O_RDONLY);
  if(fd1 < 0) {
    err(1, "Couldn't open file");
  }

  int fd2 = open(argv[2], O_RDONLY);
  if(fd2 < 0) {
    close(fd1);
    err(1, "Couldn't open file");
  }

  int fd3 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd3 < 0) {
    close(fd1);
    close(fd2);
    err(1, "Couldn't open file");
  }

  uint32_t x,y;
  int bytesx, bytesy;
  while(true) {
    bytesx = read(fd1, &x, sizeof(x));
    if(bytesx <= 0) {
      break;
    }

    bytesy = read(fd1, &y, sizeof(y));
    if(bytesy <= 0) {
      break;
    }
    
    if(write_interval(x,y,fd2,fd3)) {
      close(fd1);
      close(fd2);
      close(fd3);
      err(1, "Error while writing interval");
    }
  }

  if(bytesx < 0 || bytesy < 0) {
    close(fd1);
    close(fd2);
    close(fd3);
    err(1, "Error while reading from file");
  }

  close(fd1);
  close(fd2);
  close(fd3);  

  return 0;
} 
