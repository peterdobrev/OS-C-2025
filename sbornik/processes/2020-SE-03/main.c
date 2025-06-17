#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

uint16_t getNumber(const char* fileName, uint32_t offset, uint32_t length) {
  int p[2];
  if(pipe(p) < 0) err(1, "Couldn't pipe");

  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid == 0) {
    // child
    close(p[0]);

    int fd = open(fileName, )

    uint16_t res = xorAllData(fd);
  }
}

int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "bad args");
    
  int fd = open(argv[1], O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open");
  }


}
