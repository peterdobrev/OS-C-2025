#include <err.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

void printNum(int fd, uint16_t num) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%d", num);
  int written = write(fd, buf, strlen(buf));
  if(written < 0) err(1, "Couldn't write");
  if(written != strlen(buf)) errx(1, "Couldn't write all");
}

void handleRecord(const char* name, uint32_t offset, uint32_t length, int pipeEnd) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid != 0) {
    close(pipeEnd);
    return;
  }

  int fd = open(name, O_RDONLY);
  if(fd < 0) err(1, "Couldn't open");

  if(lseek(fd, offset * sizeof(uint16_t), SEEK_SET) < 0) err(1, "Couldn't lseek");
  
  uint16_t res = 0;

  int bytes;
  uint16_t cur;
  for(int i = 0; i < length; i++) {
    bytes = read(fd, &cur, sizeof(cur));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(cur)) err(1, "Couldn't read all");

    res = res ^ cur;
  }

  if(write(pipeEnd, &res, sizeof(res)) < 0) err(1, "Couldn't write");

  close(pipeEnd);
  close(fd);
  exit(0);
}

int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "bad args");
    
  int fd = open(argv[1], O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open");
  }

  int pipes[8][2];
  int pipeInd = 0;

  int bytes;
  char name[8];
  uint32_t offset, length;

  for(int i = 0; i < 8; i++) {
    bytes = read(fd, &name, sizeof(name));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) break;
    if(bytes != sizeof(name)) errx(1, "Couldn't read all");
  
    bytes = read(fd, &offset, sizeof(offset));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(offset)) errx(1, "Couldn't read all");

    bytes = read(fd, &length, sizeof(length));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(length)) errx(1, "Couldn't read all");
    
    if(pipe(pipes[pipeInd++]) < 0) err(1, "Couldn't pipe");

    handleRecord(name, offset, length, pipes[pipeInd - 1][1]);
  } 
  int status;
  for(int i = 0; i < pipeInd; i++) {
    wait(&status);
    if(!WIFEXITED(status)) err(1, "Child exited with error status");
  }
 
  uint16_t res = 0;
  uint16_t cur;
  for(int i = 0; i < pipeInd; i++) {
    bytes = read(pipes[i][0], &cur, sizeof(cur));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(cur)) errx(1, "Couldn't read all");
    res = res ^ cur;
  }

  printNum(1, res);
}
