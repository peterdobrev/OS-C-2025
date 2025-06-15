#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <stdio.h>

void print(int fd, const char* message) {
  int written = write(fd, message, strlen(message));
  if(written < 0) {
    err(1, "Couldn't write");
  }
  if(written != strlen(message)) {
    errx(1, "Couldn't write whole message");
  }
}

void printUint32(int fd, uint32_t num) {
  // max number is 524288 according to problem description
  char buf[7];
  snprintf(buf, sizeof(buf), "%d", num);
  print(fd, buf);
}

void printUint16(int fd, uint16_t num) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%d", num);
  print(fd, buf);
}

void printData(int fd_from, int fd_to) {
  int bytes;
  uint16_t cur;
  while((bytes = read(fd_from, &cur, sizeof(cur))) > 0) {
    if(bytes != sizeof(cur)) errx(1, "Couldn't read all");
    printUint16(fd_to, cur);
    print(fd_to, ", ");
  }
  if(bytes < 0) {
    err(1, "Couldn't read");
  }
}

int main(int argc, char* argv[]) {
  if(argc != 3) errx(1, "Bad args");
  
  int fd_input = open(argv[1], O_RDONLY);
  if(fd_input < 0) err(1, "Couldn't open");
  
  int fd_output = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(fd_output < 0) err(1, "Couldn't open");

  struct stat file_info; 
  if(fstat(fd_input, &file_info) < 0) {
    err(1, "Couldn't stat");
  }

  if(file_info.st_size % 2 != 0) {
    errx(1, "Wrong file format");
  }

  uint32_t fileSize = file_info.st_size / sizeof(uint16_t);

  const char mess1[] = "#include <stdint.h>\n\n";
  const char mess2[] = "const uint16_t arr[] = { ";
  const char mess2End[] = " };\n";
  const char mess3[] = "const uint32_t arrN = ";
  const char mess3End[] = ";";

  print(fd_output, mess1);

  print(fd_output, mess2);
  printData(fd_input, fd_output);
  print(fd_output, mess2End);
  
  close(fd_input);

  print(fd_output, mess3);
  printUint32(fd_output, fileSize);
  print(fd_output, mess3End);

  close(fd_output);

  return 0;
}
