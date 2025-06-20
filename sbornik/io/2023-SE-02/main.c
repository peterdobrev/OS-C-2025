#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <string.h>

void printMessage(int fd, const char* message) {
  int len = strlen(message);
  int written = write(fd, message, len);
  if(written < 0) err(1, "Couldn't write");
  if(written != len) errx(1, "Couldn't write all");
}

void extract(const char* str, char* buf, int size, char sep) {
  int ind = 0;
  int i = 0;
  int len = strlen(str);
  while(i < len && ind < size - 1) {
    if(str[i] == sep) break;
    buf[ind++] = str[i++];
  }
  buf[ind] = '\0';
} 

void getline(int fd, char* buf, int size, char sep) {
  int bytes;
  char cur;
  int ind = 0;
  while((bytes = read(fd, &cur, sizeof(cur))) > 0 && ind < size - 1) {
    if(cur == sep) break;
    buf[ind++] = cur;
  }
  buf[ind] = '\0';
  if(bytes < 0) err(1, "Couldn't read");
}

void findAllIdx(int fd, uint32_t* idxArr) {
  if(lseek(fd, 0, SEEK_SET) < 0) err(1, "Couldn't lseek");

  int idx = 0;
  char c;
  int bytes;
  uint32_t pos = 0;
  while((bytes = read(fd, &c, sizeof(c))) > 0) {
    // incrementing the pos first will fill the array with the first positions which are not null
    pos++;
    if(c == '\0') idxArr[idx++] = pos;
  }
  if(bytes < 0) err(1, "Couldn't read");
}

int countElements(int fd) {
  if(lseek(fd, 0, SEEK_SET) < 0) err(1, "Couldn't lseek");

  char c;
  int bytes;
  int counter = 0;
  while((bytes = read(fd, &c, sizeof(c))) > 0) {
    if(c == '\0') counter++;
  }
  if(bytes < 0) err(1, "Couldn't read");

  return counter;
}

void getElementAt(int fd, uint32_t* idxArr, int idx, char* buf, int bufSize) {
  uint32_t pos = idxArr[idx];
  if(lseek(fd, pos, SEEK_SET) < 0) err(1, "Couldn't set pos");

  getline(fd, buf, bufSize, '\0');
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "Bad args");

  char* toFind = argv[1];

  int fd = open(argv[2], O_RDONLY);
  if(fd < 0) err(1, "Couldn't open");

  int n = countElements(fd);

  uint32_t* idxArr = malloc(n * sizeof(uint32_t));

  findAllIdx(fd, idxArr);
  
  bool found = false;
  int l = 0, r = n - 1;
  while(l<=r) {
    int m = l + (r-l) / 2;
    
    char buf[4096];
    getElementAt(fd, idxArr, m, buf, sizeof(buf));
    
    char word[65];
    extract(buf, word, sizeof(word), '\n');

    if(strcmp(word, toFind) < 0) {
      l = m + 1;
    } else if(strcmp(word, toFind) > 0) {
      r = m - 1;
    } else {
      found = true;
      char* desc = strchr(buf, '\n');
      desc++;
      printMessage(1, desc);
      break;
    }
  }
  
  free(idxArr);
  close(fd);

  if(!found) {
    printMessage(1, "Couldn't find word in dictionary");
  }

  return 0;
}
