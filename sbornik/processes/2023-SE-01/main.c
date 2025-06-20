#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

void findAllFiles(int findpipe[2], const char* dir) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid == 0) {
    // child
    close(findpipe[0]);
    dup2(findpipe[1], 1);
    close(findpipe[1]);

    char command[1024];
    command[0] = '\0';

    strcat(command, "find ");
    strcat(command, dir);

    strcat(command, " -type f -not -name '*.hash'");

    execlp("/bin/sh", "sh", "-c", command, NULL);

    err(1, "Couldn't execlp");
  } else {
    close(findpipe[1]);
  }
}

void getline(int fd, char buf[1024], int size, char sep) {
  int bytes;
  char cur;
  int ind = 0;
  while((bytes = read(fd, &cur, sizeof(cur))) > 0 && ind < size - 1) {
    if(cur == sep) break;
    buf[ind++] = cur;
  }
  if(bytes < 0) err(1, "Couldn't read");
  
  buf[ind] = '\0';
}

void getChecksum(int fd, const char* name) {
  dup2(fd, 1);
  close(fd);
  execlp("md5sum", "md5sum", name, NULL);
  err(1, "Couldn't md5sum");
}

void createChecksum(char* name) {
  char fileName[2048];
  fileName[0] = '\0';
  strcat(fileName, name);
  strcat(fileName, ".hash");

  int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd < 0) err(1, "Couldn't open");

  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");
  
  if(pid == 0) getChecksum(fd, name);
  else close(fd);
  
  int status;
  wait(&status);

  if(!WIFEXITED(status)) {
    err(1, "Child exited with error status");
  }

  exit(0);
}

int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "bad args");
  
  int findpipe[2];
  if(pipe(findpipe) < 0) err(1, "Couldn't pipe");

  findAllFiles(findpipe, argv[1]);

  char buf[1024];
  int childrenCount = 0;
  while(1) {
    getline(findpipe[0], buf, sizeof(buf), '\n');
    if(strlen(buf) == 0) break;
    
    childrenCount++;
    pid_t pid = fork();
    if(pid < 0) err(1, "Couldn't fork");
    if(pid == 0) {
      // child
      close(findpipe[0]);
      createChecksum(buf);
    }
  }

  close(findpipe[0]);

  for(int i = 0; i < childrenCount + 1; i++) {
    int status;
    wait(&status);
    if(!WIFEXITED(status)) {
      err(1, "Child exited with error status");
    }
  }

  return 0;

}
