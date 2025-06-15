#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>

uint16_t encode(uint8_t a) {
  uint16_t res = 0;
  for(int i = 0; i < 8; i++) {
    if(a & (1 << i)) {
      res |= (1 << (2*i+1));
    } else {
      res |= (1 << (2*i));
    }
  }

  return res;
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "bad args");

  int fd_input = open(argv[1], O_RDONLY);
  if(fd_input < 0) err(1, "Couldn't open");

  int fd_output = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_output < 0) err(1, "Couldn't open");

  int bytes;
  uint8_t cur;
  while((bytes = read(fd_input, &cur, sizeof(cur))) > 0) {
    uint16_t encoded_data = encode(cur);
    int written = write(fd_output, &encoded_data, sizeof(encoded_data));
    if(written < 0) err(1, "Couldn't read");
    if(written != sizeof(encoded_data)) errx(1, "Couldn't write all");
  }
  if(bytes < 0) err(1, "Couldn't read");

  close(fd_input);
  close(fd_output);
}
