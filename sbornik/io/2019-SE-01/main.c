#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <err.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>

void printNumber(uint32_t number) {
  char buf[15];
  snprintf(buf, sizeof(buf), "%d", number);
  write(1, buf, strlen(buf));
}

void insert(uint32_t* uids, uint32_t* times, int* curInd, int size, uint32_t uid, uint32_t time) {
  bool found = false;
  for(int i = 0; i < size; i++) {
    if(times[i] == 0) break;

    if(uids[i] == uid) {
      found = true;
      if(times[i] < time) {
        times[i] = time;
      }
    }
  }

  if(!found) {
    uids[*curInd] = uid;
    times[*curInd] = time;
    (*curInd)++;
  }
}

void printAboveDispersion(uint32_t* uids, uint32_t* times, int size, double dispersion) {
  for(int i = 0; i < size; i++) {
    if(times[i] == 0) break;

    if((double)(times[i] * times[i]) > dispersion) {
      printNumber(uids[i]);
      if(write(1, "\n", 1) < 0) err(1, "Couldn't write");
    }
  }
}



int main(int argc, char* argv[]) {
  if(argc != 2) errx(1, "bad args");

  int fd = open(argv[1], O_RDONLY);

  struct stat file_info;
  if(fstat(fd, &file_info) < 0) {
    err(1, "Couldn't stat");
  }

  int size = file_info.st_size;
  if(size % 16 != 0) {
    errx(1, "Wrong file format");
  }

  uint32_t uids[2048];
  uint32_t times[2048];
  for(int i = 0; i < sizeof(uids) / sizeof(uids[0]); i++) {
    uids[i] = 0;
    times[i] = 0;
  }

  int bytes;
  int curInd = 0;

  uint32_t records[16384];
  int recInd = 0;
  uint32_t totalSum = 0;

  while(1) {
    uint32_t uid;
    bytes = read(fd, &uid, sizeof(uid));
    if(bytes == 0) break;
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(uid)) errx(1, "Couldn't read enough bytes");
    
    uint16_t dummy;
    for(int j = 0; j < 2; j++) {
      bytes = read(fd, &dummy, sizeof(dummy));
      if(bytes < 0) err(1, "Couldn't read");
      if(bytes != sizeof(dummy)) errx(1, "Couldn't read enough bytes");
    }

    uint32_t start, end;
    bytes = read(fd, &start, sizeof(start));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(start)) errx(1, "Couldn't read enough bytes");
    
    bytes = read(fd, &end, sizeof(end));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(end)) errx(1, "Couldn't read enough bytes");

    uint32_t time = end - start;
    records[recInd++] = time;
    totalSum += time;
    insert(uids, times, &curInd, (sizeof uids / sizeof uids[0]), uid, time);
  }

  double avg = ((double) totalSum) / recInd;

  double dispSum = 0;
  for(int i = 0; i < recInd; i++) {
    dispSum += (records[i] - avg) * (records[i] - avg);
  }

  double dispersion = dispSum / recInd;

  printAboveDispersion(uids, times, (sizeof uids / sizeof uids[0]), dispersion);

  close(fd);
}
