#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  uint32_t magic;
  uint32_t packet_count;
  uint64_t originalSize;
} header;

void printMessage(int fd, const char* message) {
  int len = strlen(message);
  int written = write(fd, message, len);
  if(written < 0) err(1, "Couldn't write");
  if(written != len) errx(1, "Couldn't write all");
}

void printByte(int fd, uint8_t byte) {
  int written = write(fd, &byte, sizeof(byte));
  if(written < 0) err(1, "Couldn't write");
  if(written != sizeof(byte)) errx(1, "Couldn't write all");
}

int getType(uint8_t byte) {
  return byte & (1 << 7);
} 

int getN(uint8_t byte) {
  return byte &= (1 << 7);
}

void writeNextNBytes(int fd_from, int fd_to, int n) {
  int bytes;
  uint8_t cur;
  int i = 0;
  while((bytes = read(fd_from, &cur, sizeof(cur))) > 0 && i < n) {
    printByte(fd_to, cur);
    i++;
  }
  if(bytes < 0) err(1, "Couldn't read");
}

void writeNextByteNtimes(int fd_from, int fd_to, int n) {
  int bytes;
  uint8_t cur;
  bytes = read(fd_from, &cur, sizeof(cur));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes == 0) errx(1, "Couldn't read a byte");
  
  for(int i = 0; i < n; i++) {
    printByte(fd_to, cur);
  }
}

int main(int argc, char* argv[]) {
  // ./main compressed original
  if(argc != 3) err(1, "bad args");

  int fd_from = open(argv[1], O_RDONLY);
  if(fd_from < 0) err(1, "Couldn't open");

  int fd_to = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_to < 0) err(1, "Couldn't open");

  header h;
  int bytes;
  bytes = read(fd_from, &h, sizeof(h));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes != sizeof(h)) errx(1, "Couldn't read all");

  if(h.magic != 0x21494D46) errx(1, "Bad format");
  
  uint8_t cur;
  while((bytes = read(fd_from, &cur, sizeof(cur))) > 0) {
    if(getType(cur) == 0) writeNextNBytes(fd_from, fd_to, getN(cur));
    else writeNextByteNtimes(fd_from, fd_to, getN(cur)); 
  }
  if(bytes < 0) err(1, "Couldn't read");

  close(fd_from);
  close(fd_to);

  return 0;
}
