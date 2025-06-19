#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>

void loadIntoFile(int fd, uint8_t* arr, ssize_t len) {
  for(int i = 0; i < len; i++) {
    int written = write(fd, &arr[i], sizeof(uint8_t));
    if(written < 0) {
      err(1, "Couldn't write");
    } 

    if(written == 0) errx(1, "Couldn't write all");
  }
}

void handleMessage(int fd_from, int fd_to) {
  uint8_t N;
  int bytes = read(fd_from, &N, sizeof(N));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes == 0) errx(1, "Bad format");

  if(N < 2) return;

  uint8_t checkSum = 0;
  uint8_t cur;
  uint8_t buf[256];
  int ind = 0;
  for(int i = 3; i <= N - 1; i++) {
    bytes = read(fd_from, &cur, sizeof(cur));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) errx(1, "bad format");

    buf[ind++] = cur;
    checkSum ^= cur;
  } 

  uint8_t realCheckSum;

  bytes = read(fd_from, &realCheckSum, sizeof(realCheckSum));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes == 0) errx(1, "bad format");


  if(realCheckSum != checkSum) errx(1, "bad format");

  loadIntoFile(fd_to, buf, ind);
}

int main(int argc, char* argv[]) {
  if(argc != 3) err(1, "bad args");

  int fd_from = open(argv[1], O_RDONLY);
  if(fd_from < 0) err(1, "Couldn't open");

  int fd_to = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(fd_to < 0) err(1, "Couldn't open");

  int bytes;
  uint8_t cur;
  while(1) {
    bytes = read(fd_from, &cur, sizeof(cur));
    if(bytes == 0) break;
    if(bytes < 0) err(1, "Couldn't read");

    if(cur == 0x55) handleMessage(fd_from, fd_to);
  }

  close(fd_from);
  close(fd_to);

  return 0;
}
