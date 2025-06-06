#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
  execl("/bin/sleep", "sleep", "3", NULL);

  char* args[] = {"sleep", "3", NULL};
  execv("bin/sleep", args);

  execlp("sleep", "sleep", "3", NULL);
  execvp("sleep", args);
}
