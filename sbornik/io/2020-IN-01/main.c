#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <err.h>

void replaceWord(int fd, uint32_t offset, uint16_t old, uint16_t new) {
  if(lseek(fd, offset, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }

  uint16_t cur;
  if(read(fd, &cur, sizeof(cur)) < 0) {
    err(1, "Couldn't read");
  }

  if(cur != old) {
    errx(1, "Wrong file format");
  }

  if(lseek(fd, offset, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }
  
  if(write(fd, &new, sizeof(new)) < 0) {
    err(1, "Couldn't write");
  }
}

void replaceByte(int fd, uint16_t offset, uint8_t old, uint8_t new) {
  if(lseek(fd, offset, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }

  uint8_t cur;
  if(read(fd, &cur, sizeof(cur)) < 0) {
    err(1, "Couldn't read");
  }

  if(cur != old) {
    errx(1, "Wrong file format");
  }

  if(lseek(fd, offset, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }
  
  if(write(fd, &new, sizeof(new)) < 0) {
    err(1, "Couldn't write");
  }
}

void copyFiles(int from_fd, int to_fd) {
  if(lseek(from_fd, 0, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }

  if(lseek(to_fd, 0, SEEK_SET) < 0) {
    err(1, "Couldn't set pos");
  }

  char buf[4096];
  int bytes;
  while((bytes = read(from_fd, buf, sizeof(buf))) > 0) {
    int written = write(to_fd, buf, bytes);
    if(written < 0) {
      err(1, "Couldn't write");
    }
    if(written != bytes) {
      errx(1, "Couldn't write all bytes");
    }
  }

  if(bytes < 0) {
    err(1, "Couldn't read");
  }
}

void handleNewVersion(int patch_fd, int f2_fd) {
  int bytes;
  uint32_t offset;
  uint16_t old, new;

  while(1) {
    bytes = read(patch_fd, &offset, sizeof(offset));
    if(bytes == 0) break;
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(offset)) errx(1, "Couldn't read all");
    
    bytes = read(patch_fd, &old, sizeof(old));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(old)) errx(1, "Couldn't read all");
    
    bytes = read(patch_fd, &new, sizeof(new));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(new)) errx(1, "Couldn't read all");
    
    replaceWord(f2_fd,offset, old, new);
  }
}

void handleOldVersion(int patch_fd, int f2_fd) {
  int bytes;
  uint16_t offset;
  uint8_t old, new;

  while(1) {
    bytes = read(patch_fd, &offset, sizeof(offset));
    if(bytes == 0) break;
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(offset)) errx(1, "Couldn't read all");
    
    bytes = read(patch_fd, &old, sizeof(old));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(old)) errx(1, "Couldn't read all");
    
    bytes = read(patch_fd, &new, sizeof(new));
    if(bytes < 0) err(1, "Couldn't read");
    if(bytes != sizeof(new)) errx(1, "Couldn't read all");
    
    replaceByte(f2_fd,offset, old, new);
  }
}

int main(int argc, char* argv[]) {
  if(argc != 4) errx(1, "bad args");

  int patch_fd = open(argv[1], O_RDONLY);
  if(patch_fd < 0) err(1, "Couldn't read");

  int f1_fd = open(argv[2], O_RDONLY);
  if(f1_fd < 0) err(1, "Couldn't read");

  int f2_fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(f2_fd < 0) err(1, "Couldn't read");
  
  struct stat file_info;
  if(fstat(patch_fd, &file_info) < 0) err(1, "Couldn't stat");

  if(file_info.st_size < 16) err(1, "Wrong file format");

  uint8_t data_version;
  uint16_t count;

  // skip unnecessary info
  uint8_t dummy;
  for(int i = 0; i < 5; i++) {
    if(read(patch_fd, &dummy, 1) < 0) err(1, "Couldn't read");
  }

  if(read(patch_fd, &data_version, sizeof(data_version)) < 0) err(1, "Couldn't read");

  if(read(patch_fd, &count, sizeof(count)) < 0) err(1, "Couldn't read");

  if(data_version == 0x00) {
    if(file_info.st_size % 4 != 0) errx(1, "Wrong file format");
  } else {
    if(file_info.st_size % 8 != 0) errx(1, "Wrong file format");
  }

  copyFiles(f1_fd, f2_fd);
  
  close(f1_fd);

  if(data_version == 0x00) {
    handleOldVersion(patch_fd, f2_fd);
  } else {
    handleNewVersion(patch_fd, f2_fd);
  }

  close(patch_fd);
  close(f2_fd);
}
