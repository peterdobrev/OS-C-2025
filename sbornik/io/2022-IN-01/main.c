#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>

void skipHeader(int fd) {
  if(lseek(fd, 8, SEEK_SET) < 0) err(1, "Couldn't lseek");
}

void skipToElement(int fd, ssize_t element, ssize_t elementSize) {
  skipHeader(fd);
  if(lseek(fd, element * elementSize, SEEK_CUR) < 0) err(1, "Couldn't lseek");
}

void findType(char* types[3], const char* filename) {
  int fd = open(filename, O_RDONLY);
  if(fd < 0) err(1, "Couldn't open");

  int bytes;
  uint16_t magic;
  uint16_t filetype;
  
  bytes = read(fd, &magic, sizeof(magic));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes != sizeof(magic)) errx(1, "Couldn't read all");
  if(magic != 0x5A4D) errx(1, "File not from Hoge");

  bytes = read(fd, &filetype, sizeof(filetype));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes != sizeof(filetype)) errx(1, "Couldn't read all");
  if(filetype > 3) errx(1, "File wrong format");
  if(types[filetype - 1] != NULL) errx(1, "Passed two files from same type");
  types[filetype - 1] = filename;

  close(fd);
}

void replace(int data_fd, int out_fd, uint16_t pos_data, uint16_t pos_out) {
  skipToElement(data_fd, pos_data, sizeof(uint32_t));
  skipToElement(out_fd, pos_out, sizeof(uint64_t));

  uint32_t element;
  int bytes = read(data_fd, &element, sizeof(element));
  if(bytes < 0) err(1, "Couldn't read");
  if(bytes != sizeof(element)) errx(1, "Couldn't read all");

  uint64_t out_element = element;
  int written = write(out_fd, &out_element, sizeof(out_element));
  if(written < 0) err(1, "Couldn't write");
  if(written != sizeof(out_element)) err(1, "Couldn't write all");
}

void constructHeader(int out_fd, uint32_t count) {
  if(lseek(out_fd, 0, SEEK_SET) < 0) {
    err(1, "Couldn't lseek");
  }
  
  uint16_t magic = 0x5A4D;
  uint16_t filetype = 3;
  if(write(out_fd, &magic, sizeof(magic)) < 0) err(1, "Couldn't write");
  if(write(out_fd, &filetype, sizeof(filetype)) < 0) err(1, "Couldn't write");
  if(write(out_fd, &count, sizeof(count)) < 0) err(1, "Couldn't write");
}

int main(int argc, char* argv[]) {
  if(argc != 4) errx(1, "bad args");

  char* types[3];
  for(int i = 0; i < 3; i++) types[i] = NULL;
  for(int i = 1; i <= 3; i++) findType(types, argv[i]);

  int list_fd = open(types[0], O_RDONLY);
  if(list_fd < 0) err(1, "Couldn't open");

  int data_fd = open(types[1], O_RDONLY);
  if(data_fd < 0) err(1, "Couldn't open");

  int out_fd = open(types[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(out_fd < 0) err(1, "Couldn't open");

  int bytes;
  uint16_t pos_data;
  uint16_t pos_out;

  uint32_t outDataCounter = 0;

  while(1) {
    bytes = read(list_fd, &pos_data, sizeof(pos_data));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) break; // eof
    if(bytes != sizeof(pos_data)) errx(1, "Couldn't read full");
  
    bytes = read(list_fd, &pos_out, sizeof(pos_out));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes == 0) break; // eof
    if(bytes != sizeof(pos_out)) errx(1, "Couldn't read full");

    replace(data_fd, out_fd, pos_data, pos_out);
    outDataCounter++;
  }

  close(list_fd);
  close(data_fd);
  
  constructHeader(out_fd, outDataCounter);

  close(out_fd);
  
  return 0;
}
