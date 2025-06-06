#include <stddef.h>
#include <unistd.h>
int main() {
  execlp("date", "date", NULL);
  
  // char* args[] = {"date", NULL};
  // execvp("date", args);
  
  // execl("/bin/date", "date", NULL);
  // char* args[] = {"date", NULL};
  
  // execv("bin/date", args);
}
