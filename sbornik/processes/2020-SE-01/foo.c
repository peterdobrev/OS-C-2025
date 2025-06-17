#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "bad args");

  const char* pipe_name = "/tmp/foo_bar_pipe";

  if(mkfifo(pipe_name, 0666) == -1 && errno != EEXIST) {
    err(1, "Couldn't create pipe");
  }

  int fd = open(pipe_name, O_WRONLY);
  if(fd < 0) err(1, "Couldn't open");

  dup2(fd, 1);
  close(fd);

  execlp("cat", "cat", argv[1], NULL);
  err(1, "Couldn't execlp");
}
