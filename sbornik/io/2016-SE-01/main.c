#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
  if(argc != 2) {
    errx(1, "Expected file");
  }

  int fd = open(argv[1], O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open file:");
  }

  struct stat file;
  if(fstat(fd, &file) < 0) {
    close(fd);
    err(1, "Couldn't stat file:");
  }

  int size = file.st_size;
  uint8_t* arr = malloc(size * sizeof(uint8_t));

  int bytes = read(fd, arr, size);
  if(bytes < 0) {
    close(fd);
    err(1, "Couldn't read from file");
  }

  if(bytes != size) {
    close(fd);
    errx(1, "Couldn't read whole file");
  }

  close(fd);

  uint64_t* mp = malloc(256 * sizeof(uint64_t));
  for(int i = 0; i < 256; i++) {
    mp[i] = 0;
  }

  for(int i = 0; i < bytes; i++) {
    mp[arr[i]]++;
  }

  int x = 0;
  for(int i = 0; i < 256; i++) {
    for(int j = 0; j < mp[i]; j++) {
      arr[x++] = i;
    }
  }

  fd = open(argv[1], O_WRONLY | O_TRUNC);
  if(fd < 0) {
    err(1, "Couldn't open file");
  }

  int written = write(fd, arr, bytes);
  if(written < 0) {
    close(fd);
    err(1, "Error when writing to file");
  }

  if(written != bytes) {
    close(fd);
    errx(1, "Couldn't write all bytes");
  }

  close(fd);
  free(arr);
  free(mp);

  return 0;
}
