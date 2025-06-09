#include <string.h>
#include <err.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

void runComm(const char* comm, char* args[]) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid == 0) {
    execvp(comm, args);
    errx(1, "Couldn't exec command %s", comm);
  }

  wait(NULL);
}

bool validStr(const char* str) {
  if(strlen(str) > 10) return false;

  int argC = 1;
  int charC = 0;
  while(*str) {
    if(*str == ' ' || *str == '\n') {
      argC++;
      charC = 0;
    } else {
      charC++;
      if(charC > 4) {
        return false;
      }
    }
    str++;
  }

  if(argC > 2) {
    return false;
  }

  return true;
}

void copyTo(const char* from, char* to, int spacesSkip) {
  // skip leading spaces
  while(*from && (*from == ' ' || *from == '\n')) {
    from++;
  }

  for(int i = 0; i < spacesSkip; i++) {
    while(*from) {
      if(*from == ' ' || *from == '\n') {
        from++;
        break;
      }
      from++;
    }
  }

  int total = 0;
  while(*from && (*from != ' ' || *from != '\n') && total < 4) {
    *to = *from;
    from++;
    to++;
    total++;
  }
  *to = '\0';
}

int main(int argc, char* argv[]) {
  if(argc > 2) errx(1, "bad args");

  char* command;
  char def_command[] = "echo";
  if(argc == 2) {
    if(strlen(argv[1]) > 4) errx(1, "bad args");
    command = argv[1];
  } else {
    command = def_command;
  }

  char buf[1024];
  int bytes;
  while((bytes = read(0, buf, sizeof(buf))) > 0) {
    buf[bytes] = '\0';
    if(!validStr(buf)) {
      errx(1, "bad args");
    }

    char par1[5];
    char par2[5];
    copyTo(buf, par1, 0);
    copyTo(buf, par2, 1);

    char* args[] = {command, par1, par2, NULL};
    runComm(command, args);
  }
  if(bytes < 0) {
    err(1, "Couldn't read");
  }

  return 0;
}
