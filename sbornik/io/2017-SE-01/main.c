#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
  if(argc != 4) {
    errx(1, "Bad args");
  }
  int fd1 = open(argv[1], O_RDONLY);
  if(fd1 < 0) {
    err(1, "Couldn't open");
  }
  int fd2 = open(argv[2], O_RDONLY);
  if(fd2 < 0) {
    err(1, "Couldn't open");
  }
  int fd_patch = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_patch < 0) {
    err(1, "Couldn't open");
  }
  
  int bytes;
  uint8_t b1, b2;
  uint16_t offset = 0;
  int end1 = 0, end2 = 0;
  
  while(1) {
    bytes = read(fd1, &b1, 1);
    if(bytes < 0) {
      err(1, "Couldn't read");
    }
    if(bytes == 0) {
      end1 = 1;
    }
    
    bytes = read(fd2, &b2, 1);
    if(bytes < 0) {
      err(1, "Couldn't read");
    }
    if(bytes == 0) {
      end2 = 1;
    }
    
    if(end1 != end2) {
      errx(1, "files have different sizes");
    } else if(end1) {
      break;
    }
    
    if(b1 != b2) {
      if(write(fd_patch, &offset, sizeof(offset)) < 0) {
        err(1, "Couldn't write");
      }
      if(write(fd_patch, &b1, sizeof(b1)) < 0) {
        err(1, "Couldn't write");
      }
      if(write(fd_patch, &b2, sizeof(b2)) < 0) {
        err(1, "Couldn't write");
      }
    }
    
    offset++;
  }
  
  close(fd1);
  close(fd2);
  close(fd_patch);
  
  return 0;
}
