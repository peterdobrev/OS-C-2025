#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <err.h>

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    err(1, "Error when forking");
  }

  if(pid > 0) {
    // parent
    int status;
    pid_t waited_pid = wait(&status);
    
    if (waited_pid < 0) {
      err(1, "Couldn't wait to finish");
    }
    if (!WIFEXITED(status)) {
      errx(1, "task failed: child was killed!");
    } else if (WEXITSTATUS(status) != 0) {
      errx(1, "task failed (exit status != 0)!");
    }

    const char msg[] = "I am your father\n";
    write(1, msg, strlen(msg));
  } else {
    // child
    const char msg[] = "Nooooooo!\n";
    write(1, msg, strlen(msg));
  }
}
