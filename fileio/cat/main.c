#include <fcntl.h>
#include <err.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  for(int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if(fd < 0) {
      err(1, "Couldn't open file %s", argv[i]);
    }
    char buf[1024];
    int bytes;
    while((bytes = read(fd, buf, sizeof(buf))) > 0) {
      write(1, buf, bytes);
    }
    if(bytes < 0) {
      close(fd);
      err(1, "Error when reading from %s", argv[i]);
    }
    close(fd);
  }
  return 0;
}
