#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    errx(1, "Pass a file");
  }

  int pfd[2];
  if(pipe(pfd) < 0) {
    err(1, "Couldn't pipe");
  }

  int pid = fork();

  if(pid < 0) {
    err(1, "Couldn't fork");
  }

  if(pid == 0) {
    // child
    close(pfd[0]);
    dup2(pfd[1], 1);
    close(pfd[1]);
    execlp("cat", "cat", argv[1], NULL);
    errx(1, "Couldn't execute cat");
  } else {
    // parent
    close(pfd[1]);
    dup2(pfd[0], 0);
    close(pfd[0]);
    execlp("sort", "sort", NULL);
    errx(0, "Couldn't execute sort");
  }
}
