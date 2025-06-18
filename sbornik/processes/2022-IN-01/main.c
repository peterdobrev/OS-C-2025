#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int printMessage(int fd, const char* message) {
  int written = write(fd, message, strlen(message));
  if(written < 0) err(1, "Couldn't write");
  if(written != strlen(message)) errx(1, "Couldn't write");
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "bad args");

  int parent_child[2], child_parent[2];
  if(pipe(parent_child) < 0) err(1, "Couldn't pipe");
  if(pipe(child_parent) < 0) err(1, "Couldn't pipe");
  
  int N = strtol(argv[1], NULL, 10);  
  int D = strtol(argv[2], NULL, 10);

  char parentMessage[] = "DING\n";
  char childMessage[] = "DONG\n";
  
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");
  
  if(pid == 0) {
    // child
    close(parent_child[1]);
    close(child_parent[0]);
  } else {
    // parent
    close(parent_child[0]);
    close(child_parent[1]);
  }

  for(int i = 0; i < N; i++) {
    if(pid == 0) {
      char dummy;
      int bytes = read(parent_child[0], &dummy, sizeof(dummy));
      if(bytes < 0) err(1, "Couldn't read");
      if(bytes == 0) errx(1, "Expected info from parent");

      printMessage(1, childMessage);
      printMessage(child_parent[1], "0");
    } else {
      printMessage(1, parentMessage);
      printMessage(parent_child[1], "0");

      char dummy;
      int bytes = read(child_parent[0], &dummy, sizeof(dummy)); 
      if(bytes < 0) err(1, "Couldn't read");
      if(bytes == 0) errx(1, "Expected info from child");

      sleep(D);
    }
  }

  if(pid == 0) {
    // child
    close(parent_child[0]);
    close(child_parent[1]);
    exit(0);
  } else {
    // parent
    close(parent_child[1]);
    close(child_parent[0]);
  }

  int status;
  wait(&status);
  if(!WIFEXITED(status)) {
    err(1, "Child exited with error");
  }

  return 0;
}
