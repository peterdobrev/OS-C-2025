#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <stdbool.h>

void skipHeader(int fd, ssize_t headerLength) {
  if(lseek(fd, headerLength, SEEK_SET) < 0) err(1, "Couldn't lseek");
}

void skipToElement(int fd, ssize_t element, ssize_t elementSize, ssize_t headerLength) {
  skipHeader(fd, headerLength);
  if(lseek(fd, element * elementSize, SEEK_CUR) < 0) err(1, "Couldn't lseek");
}


void swap(int fd, uint32_t offset1, uint32_t offset2, bool bigger) {
  skipToElement(fd, offset1, sizeof(uint64_t), 8);
  uint64_t element1;
  int bytes = read(fd, &element1, sizeof(element1));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes != sizeof(element1)) errx(1, "Couldn't read all");

  uint64_t element2;
  skipToElement(fd, offset2, sizeof(uint64_t), 8);
  bytes = read(fd, &element2, sizeof(element2));
  if(bytes < 0 ) err(1, "Couldn't read");
  if(bytes != sizeof(element2)) errx(1, "Couldn't read all");


  if(element1 == element2) return;

  if((element1 > element2 && bigger) ||
     (element1 < element2 && !bigger)) return;

  skipToElement(fd, offset1, sizeof(uint64_t), 8);
  int written = write(fd, &element2, sizeof(element2));
  if(written < 0) err(1, "Couldn't read");
  if(written != sizeof(element2)) errx(1, "Couldn't write all");

  skipToElement(fd, offset2, sizeof(uint64_t), 8);
  written = write(fd, &element1, sizeof(element1));
  if(written < 0) err(1, "Couldn't read");
  if(written != sizeof(element1)) errx(1, "Couldn't write all");
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "bad args");

  int data_fd = open(argv[1], O_RDWR);
  if(data_fd < 0) err(1, "Couldn't open");

  int comparator_fd = open(argv[2], O_RDONLY);
  if(comparator_fd < 0) err(1, "Couldn't open");

  int bytes;
  uint32_t magic;

  if((bytes = read(data_fd, &magic, sizeof(magic))) < 0) err(1, "Couldn't read");
  if(bytes != sizeof(magic)) errx(1, "Couldn't read all");
  if(magic != 0x21796F4A) errx(1, "Wrong file format");

  uint32_t magic1;
  uint16_t magic2;

  if((bytes = read(data_fd, &magic1, sizeof(magic1))) < 0) err(1, "Couldn't read");
  if(bytes != sizeof(magic1)) errx(1, "Couldn't read all");
  if(magic != 0xAFBC7A37) errx(1, "Wrong file format");

  if((bytes = read(data_fd, &magic2, sizeof(magic2))) < 0) err(1, "Couldn't read");
  if(bytes != sizeof(magic2)) errx(1, "Couldn't read all");
  if(magic != 0x1C27) errx(1, "Wrong file format");

  skipHeader(comparator_fd, 16);

  uint16_t type;
  uint16_t dummy;
  uint32_t offset1, offset2;

  while(1) {
    bytes = read(comparator_fd, &type, sizeof(type));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) break; // eof
    if(bytes != sizeof(type)) errx(1, "Couldn't read full");
  
    for(int i = 0; i < 3; i++) {
      bytes = read(comparator_fd, &dummy, sizeof(dummy));
      if(bytes < 0) err(1, "Couldn't read");
      if(bytes != sizeof(dummy)) errx(1, "Couldn't read full");
    }

    bytes = read(comparator_fd, &offset1, sizeof(offset1));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(offset1)) errx(1, "Couldn't read full");
    
    bytes = read(comparator_fd, &offset2, sizeof(offset2));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(offset2)) errx(1, "Couldn't read full");
    
    bool bigger = type;

    swap(comparator_fd, offset1, offset2, bigger);
  }
  close(comparator_fd);
  close(data_fd);
  
  return 0;
}
