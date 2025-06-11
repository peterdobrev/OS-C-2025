#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>

int cmpfunc(const void* a, const void* b) {
  int arg1 = *(int *)a;
  int arg2 = *(int *)b;
  if(arg1 > arg2) {
    return 1;
  } else if(arg1 < arg2) {
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    errx(1, "bad args");
  }
  
  int input_fd = open(argv[1], O_RDONLY);
  if(input_fd < 0) {
    err(1, "Couldn't open input file");
  }

  int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(output_fd < 0) {
    err(1, "Couldn't open output file");
  }

  struct stat file_info;
  if(fstat(input_fd, &file_info) < 0) {
    err(1, "Couldn't stat file");
  }

  int size = file_info.st_size;
  if(size % 4 != 0) {
    errx(1, "File is not in correct format");
  }

  // get space for the ceil of size/2
  int capacity = (size / 4) * 2 * sizeof(uint32_t) + sizeof(uint32_t);
  
  uint32_t* arr = malloc(capacity);
  if(arr == NULL) err(1, "Couldn't alloc memory");

  int bytes;
  if((bytes = read(input_fd, arr, capacity)) < 0) {
    err(1, "Couldn't read");
  }
  if(bytes % sizeof(uint32_t) != 0) {
    errx(1, "Tegavo");
  }

  qsort(arr, bytes / sizeof(uint32_t), sizeof(uint32_t), cmpfunc);
 
  if(write(output_fd, arr, bytes) < 0) {
    err(1, "Couldn't write");
  }

  if((bytes = read(input_fd, arr, capacity)) < 0) {
    err(1, "Couldn't read");
  }

  if(bytes % sizeof(uint32_t) != 0) {
    errx(1, "Tegavo");
  }

  qsort(arr, bytes / sizeof(uint32_t), sizeof(uint32_t), cmpfunc);

  if(write(output_fd, arr, bytes) < 0) {
    err(1, "Couldn't write");
  }
  
  free(arr);
  close(input_fd);
  close(output_fd);
  return 0;
}
