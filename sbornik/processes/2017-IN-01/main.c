#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


int main() {
  // cut -d: -f7 /etc/passwd | sort | uniq -c | sort -n
  int pipe1[2],pipe2[2],pipe3[2];
  pid_t pid1, pid2, pid3;

  if(pipe(pipe1) < 0 || pipe(pipe2) < 0 || pipe(pipe3) < 0) {
    err(1, "Couldn't pipe");
  }

  pid1 = fork();
  if(pid1 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid1 == 0) {
    // child: cut
    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe3[0]);
    close(pipe2[1]);
    close(pipe3[1]);

    dup2(pipe1[1], 1);
    close(pipe1[1]);

    execlp("cut", "cut", "-d:", "-f7", "/etc/passwd", NULL);
    err(1, "Couldn't exec cat");
  }

  pid2 = fork();
  if(pid2 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid2 == 0) {
    // child: sort
    close(pipe2[0]);
    close(pipe3[0]);
    close(pipe1[1]);
    close(pipe3[1]);

    dup2(pipe1[0], 0);
    dup2(pipe2[1], 1);

    close(pipe2[1]);
    close(pipe1[0]);

    execlp("sort", "sort", NULL);
    err(1, "Couldn't exec sort");
  }
  pid3 = fork();
  if(pid3 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid3 == 0) {
    // child: uniq -c
    close(pipe1[0]);
    close(pipe3[0]);
    close(pipe1[1]);
    close(pipe2[1]);

    dup2(pipe2[0], 0);
    dup2(pipe3[1], 1);

    close(pipe3[1]);
    close(pipe2[0]);

    execlp("uniq", "uniq", "-c", NULL);
    err(1, "Couldn't exec uniq");
  }

  // parent
  
  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);
  close(pipe3[1]);
  dup2(pipe3[0], 0);
  close(pipe3[0]);

  execlp("sort", "sort", "-n", NULL);
  err(1, "Couldn't execute sort");
  
  wait(NULL);
  wait(NULL);
  wait(NULL);

  return 0;
}
