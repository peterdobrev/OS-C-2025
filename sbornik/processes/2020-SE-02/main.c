
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdint.h>

uint8_t decode(uint8_t a) {
  int decoded = a ^ 0x20;
  if(decoded == 0x00 || decoded == 0x55 || decoded == 0x7D || decoded == 0xFF) {
    a = decoded;
  }

  return a;
}

void decodeBytes(int fd_from, int fd_to) {
  int bytes;
  uint8_t cur;
  while((bytes = read(fd_from, &cur, sizeof(cur))) > 0) {
    if(cur == 0x55) continue;
    cur = decode(cur);
    if(write(fd_to, &cur, sizeof(cur)) < 0) {
      err(1, "Couldn't write");
    }
  }
  if(bytes < 0) {
    err(1, "Couldn't read");
  }
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "bad input");

  int p[2];
  if(pipe(p) < 0) {
    err(1, "Couldn't pipe");
  }

  int fd_to = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_to < 0) err(1, "Couldn't open");

  pid_t pid1, pid2;
  pid1 = fork();
  if(pid1 < 0) err(1, "Couldn't fork");

  if(pid1 == 0) {
    // child;
    close(p[0]);
    close(fd_to);
    dup2(p[1], 1);
    close(p[1]);

    execlp("cat", "cat", argv[1]);
    err(1, "Couldn't exec");
  }
  
  pid2 = fork();
  if(pid2 < 0) err(1, "Couldn't fork");

  if(pid2 == 0) {
    // child;
    close(p[1]);
    decodeBytes(p[0], fd_to);
    close(p[0]);
    close(fd_to);
    exit(0);
  }

  close(p[0]);
  close(p[1]);
  close(fd_to);

  int status1, status2;
  wait(&status1);
  wait(&status2);

  if(!WIFEXITED(status1) || !WIFEXITED(status2)) {
    errx(1, "Child couldn't exit normally");
  }

  close(fd_to);
}
