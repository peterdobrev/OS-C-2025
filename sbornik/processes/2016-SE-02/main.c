#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <err.h>

void handleCommand(char* command) {
  int pid = fork();
  if(pid < 0) {
    err(1, "Couldn't fork");
  }

  char commandBuf[1024];
  commandBuf[0] = '\0';
  strcat(commandBuf, "/bin/");
  strcat(commandBuf, command);

  if(pid == 0) {
    // child
    execl(commandBuf, NULL);
    errx(1, "Couldn't execute command %s", command);
  } else {
    // parent
    int status;
    waitpid(pid, &status, 0);
  }
}

int main() {
  char message[] = "Enter command: ";
  
  char buf[1024];
  int bytes;
  
  while(1) {
    int written = write(1, message, strlen(message));
    if(written < 0) {
      err(1, "Couldn't write to stdout");   
    }

    bytes = read(0, buf, sizeof(buf));
    
    if(bytes == 0) {
      continue;
    } else if(bytes < 0) {
      err(1, "Couldn't read input");
    }
    
    // replace '\n' with '\0'
    buf[bytes - 1] = '\0';

    if(strcmp(buf, "exit") == 0) {
      return 0;
    }

    handleCommand(buf);
  }
  return 0;
}
