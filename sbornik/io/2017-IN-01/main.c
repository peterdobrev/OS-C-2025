#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

int startWithCapital(int fd) {
  char letter;
  int bytes;
  bytes = read(fd, &letter, 1);
  if(bytes < 0) {
    err(1, "Couldn't read letter");
  }
  if(bytes == 0) {
    errx(1, "End of file");
  }
  int res = 0;
  if(letter >= 'A' && letter <= 'Z') res = 1;
  if(lseek(fd, -1, SEEK_CUR) < 0) {
    err(1, "Couldn't return to original pos");
  }
  return res;
}

int main(int argc, char* argv[]) {
  if(argc != 5) {
    errx(1, "Bad args");
  }
  
  int fd1_data = open(argv[1], O_RDONLY);
  if(fd1_data < 0) {
    err(1, "Couldn't open file");
  }
  int fd1_idx = open(argv[2], O_RDONLY);
  if(fd1_idx < 0) {
    err(1, "Couldn't open file");
  }
  struct stat file_info;
  if(stat(argv[2], &file_info) < 0) {
    err(1, "Couldn't stat file");
  }
  if(file_info.st_size % 4 != 0) {
    errx(1, "File not in right format");
  }
  int fd2_data = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd2_data < 0) { 
    err(1, "Couldn't open file");
  }
  int fd2_idx = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd2_idx < 0) { 
    err(1, "Couldn't open file");
  }
  
  uint16_t pos;
  uint8_t len;
  uint8_t dummy;
  int bytes;
  uint16_t lastPos = 0;
  uint8_t lastLen = 0;
  
  while(1) {
    bytes = read(fd1_idx, &pos, sizeof(pos));
    if(bytes == 0) {
      break; // End of file - normal exit
    } else if(bytes != sizeof(pos)) {
      errx(1, "Not enough bytes read");
    } else if(bytes < 0) {
      err(1, "Error while reading");
    }
    
    bytes = read(fd1_idx, &len, sizeof(len));
    if(bytes == 0 || bytes != sizeof(len)) {
      errx(1, "Not enough bytes read");
    } else if(bytes < 0) {
      err(1, "Error while reading");
    }
    
    bytes = read(fd1_idx, &dummy, sizeof(dummy));
    if(bytes != sizeof(dummy)) {
      errx(1, "Not enough bytes read");
    } else if(bytes < 0) {
      err(1, "Error while reading");
    }
    
    if(lseek(fd1_data, pos, SEEK_SET) < 0) {
      err(1, "Couldn't set pos");
    }
    
    if(!startWithCapital(fd1_data)) {
      continue;
    }
    
    uint8_t cur;
    int i = 0;
    while(i < len && (bytes = read(fd1_data, &cur, sizeof(cur))) > 0) {
      i++;
      if(write(fd2_data, &cur, sizeof(cur)) < 0) {
        err(1, "Couldn't write to file");
      }
    }
    if(bytes < 0) {
      err(1, "Couldn't read from file");
    }
    
    uint16_t writePos = lastPos + lastLen;
    
    if(write(fd2_idx, &writePos, sizeof(writePos)) < 0) {
      err(1, "Couldn't write to file");
    }
    if(write(fd2_idx, &len, sizeof(len)) < 0) {
      err(1, "Couldn't write to file");
    }
    if(write(fd2_idx, &dummy, sizeof(dummy)) < 0) {
      err(1, "Couldn't write to file");
    }
    lastLen = len;
    lastPos = writePos;
  }
  
  close(fd1_data);
  close(fd2_data);
  close(fd1_idx);
  close(fd2_idx);
  
  return 0;
}
