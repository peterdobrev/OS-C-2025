#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
  char* home = getenv("HOME");
  execl("/bin/ls", "ls", home, NULL);

  char* args[] = {"ls", home, NULL};
  execv("bin/ls", args);

  execlp("ls", "ls", home, NULL);
  execvp("ls", args);
}
