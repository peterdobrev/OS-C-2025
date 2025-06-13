#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>

// for debug
#include <stdio.h>

int cmp(const void* a, const void* b) {
  uint16_t arg1 = *(uint16_t*)a;
  uint16_t arg2 = *(uint16_t*)b;
  if(arg1>arg2) return 1;
  if(arg1<arg2) return -1;
  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "Bad args");

  int fd_input = open(argv[1], O_RDONLY);
  if(fd_input < 0) {
    err(1, "Couldn't open");
  }

  struct stat file_info;
  if(fstat(fd_input, &file_info) < 0) {
    err(1, "Couldn't stat");
  } 

  int size = file_info.st_size;
  if(size % 2 != 0) {
    errx(1, "Wrong format");
  }

  uint16_t* arr = malloc(size);
  if(arr == NULL) {
    errx(1, "Couldn't allocate memory for arr");
  }

  if(read(fd_input, arr, size) != size) {
    err(1, "Couldn't read");
  }

  close(fd_input);

  // for debug
  printf("First few bytes: %02x %02x %02x %02x\n", 
       ((uint8_t*)arr)[0], ((uint8_t*)arr)[1], 
       ((uint8_t*)arr)[2], ((uint8_t*)arr)[3]);

  qsort(arr, size / 2, sizeof(uint16_t), cmp);

  // for debug
  printf("First few bytes: %02x %02x %02x %02x\n", 
       ((uint8_t*)arr)[0], ((uint8_t*)arr)[1], 
       ((uint8_t*)arr)[2], ((uint8_t*)arr)[3]);

  int fd_output = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd_output < 0) {
    err(1, "Couldn't open");
  }

  if(write(fd_output, arr, size) != size) {
    err(1, "Couldn't save sorted arr");
  }

  close(fd_output);
  free(arr);
}
