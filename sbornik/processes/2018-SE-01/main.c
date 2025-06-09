#include <err.h>
#include <unistd.h>
#include <sys/wait.h>

// find .. -printf "%T@ %p\n" | sort -n | awk '{print $2}'

int main(int argc, char* argv[]) {
  if(argc != 2) {
    errx(1, "bad args");
  }

  int pipe1[2], pipe2[2];
  pid_t pid1, pid2, pid3;

  if(pipe(pipe1) < 0 || pipe(pipe2) < 0) {
    err(1, "Couldn't pipe");
  }

  pid1 = fork();
  if(pid1 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid1 == 0) {
    // child : find argv[1] -printf "%T@ %p\n"
    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe2[1]);
    dup2(pipe1[1], 1);
    close(pipe1[1]);
    execlp("find", "find", argv[1], "-printf", "%T@ %p\n", NULL);
    errx(1, "Couldn't exec");
  }

  pid2 = fork();
  if(pid2 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid2 == 0) {
    // child: sort -n
    close(pipe1[1]);
    close(pipe2[0]);
    dup2(pipe1[0], 0);
    dup2(pipe2[1], 1);
    close(pipe1[0]);
    close(pipe2[1]);
    execlp("sort", "sort", "-n", NULL);
    errx(1, "Couldn't exec");
  }

  pid3 = fork();
  if(pid3 < 0) {
    err(1, "Couldn't fork");
  }

  if(pid3 == 0) {
    // child: awk '{print $2}'
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[1]);
    dup2(pipe2[0], 0);
    close(pipe2[0]);

    execlp("awk", "awk", "{print $2}");
    errx(1, "Couldn't exec");
  }
  
  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);
  wait(NULL);
  wait(NULL);
  wait(NULL);

  return 0;
}
