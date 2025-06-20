// UNFINISHED

#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <stdbool.h>

bool match(const char* str, const char* pattern) {
  char* ptr = strstr(str, pattern);
  if(ptr == NULL) return false;
  return true;
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

bool runCommand(int pidPipe[2], const char* command) {
  int p[2];
  if(pipe(p) < 0) err(1, "Couldn't pipe");

  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid == 0) {
    // child
    close(p[0]);
    dup2(p[1],1);
    close(p[1]);

    execl("/bin/sh", "sh", "-c", command, NULL);
    err(1, "Couldn't execl");
  }
  else {
    // parent
    close(p[1]);

    char buf[1024];
    buf[0] = '\0';

    while(1) {
      getline(pipe[0], buf, sizeof(buf), '\n');
      if(strlen(buf) == 0) break;
      
      if(!match(buf, "found it!")) continue;
      
      kill(pid, SIGTERM);

      int status;
      wait(&status);
      if(!WIFEXITED(status)) err(1, "Child process failed");
    }   
  }
}

void createProcess(int pidPipe[2], const char* command) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid != 0) return;
  
  runCommand(pidPipe, command);

  exit(0);
}

int main(int argc, char* argv[]) {
  if(argc == 1) errx("bad args");

  for(int i = 1; i < argc; i++) {
    createProcess(argv[i]);
  }

  for(int i = 0)
}
