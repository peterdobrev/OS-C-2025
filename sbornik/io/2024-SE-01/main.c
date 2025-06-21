#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  uint64_t next; // 8 bytes
  uint8_t userData[504]; // 504 bytes
} record;

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

void addToBook(int book_fd, uint64_t x) {
  if(lseek(book_fd, x, SEEK_SET) < 0) err(1, "Couldn't lseek");
  uint8_t existsByte = 1;
  if(write(book_fd, &existsByte, sizeof(existsByte)) != sizeof(existsByte)) err(1, "Couldn't write"); 
}

int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "Bad args");

  char tmp_file_name[] = "/tmp/gpdr_XXXXXX";
  int tmp_fd = mkstemp(tmp_file_name);

  int fd = open(argv[1], O_RDWR);
  if(fd < 0) err(1, "Couldn't open");

  record cur;
  uint64_t nextPos = 0;
  while(1) {
    addToBook(tmp_fd, nextPos / sizeof(record));
    if(lseek(fd, nextPos, SEEK_SET) < 0) err(1, "Couldn't lseek");

    int bytes = read(fd, &cur, sizeof(cur));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(cur)) errx(1, "Couldn't read");

    nextPos = cur.next;
    if(nextPos == 0) break;
  }

  record emptyRecord;
  emptyRecord.next = 0;
  for(int i = 0; i < 504; i++) {
    emptyRecord.userData[i] = 0;
  }

  uint64_t posInd = 0;
  while(1) {
    uint8_t existsByte;
    int bytes = read(tmp_fd, &existsByte, sizeof(existsByte));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) break;

    if(existsByte == 1) {
      posInd++;
      continue;
    }

    uint64_t pos = posInd * sizeof(record);
    if(lseek(fd, pos, SEEK_SET) < 0) err(1, "Couldn't lseek");

    posInd++;

    int written = write(fd, &emptyRecord, sizeof(emptyRecord));
    if(written < 0) err(1, "Couldn't write");
    if(written != sizeof(emptyRecord)) errx(1, "Couldn't write");
  }

  close(fd);
  close(tmp_fd);
  unlink(tmp_file_name);
  return 0;
}
