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
    const char msg[] = "I am your father\n";
    write(1, msg, strlen(msg));
  } else {
    const char msg[] = "Nooooooo!\n";
    write(1, msg, strlen(msg));
  }
}
