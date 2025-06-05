#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

int copyfile(const char* src, const char* dest) {
  int srcfd = open(src, O_RDONLY);
  if(srcfd < 0) {
    err(1, "Cannot open %s", src);
  }

  int destfd = open(src, O_WRONLY, O_CREAT, O_TRUNC, 0644);
  if(destfd < 0) {
    close(srcfd);
    err(1, "Cannot open %s", destfd);
  }

  char buf[4096];
  int bytes;
  while((bytes = read(srcfd, buf, sizeof(buf))) > 0) {
    int written = write(destfd, buf, bytes);
    if(written != bytes) {
      close(srcfd);
      close(destfd);
      err(1, "Error writing to %s", dest);
    }
  }

  if (bytes < 0) {
    close(src_fd);
    close(dest_fd);
    err(1, "Error reading from %s", src);
  }
    
  close(src_fd);
  close(dest_fd);
}

int main(int argc, char* argv[]) {
  if(argc <= 2) {
    errx(1, "USAGE: [SOURCE]... [DIR]");
  }

  char* destdir = argv[argc-1];

  for(int i = 1; i < argc - 1; i++) {
    char* srcfile = argv[i];
    char* filename = basename(srcfile);
    char destpath[1024];
    int len = snprintf(destpath, sizeof(destpath), "%s/%s", destdir, filename);
    if(len >= sizeof(destpath)) {
      errx(1, "Destination path too long");
    }

    write(1, "Copying %s to %s\n", srcfile, destpath);
    copyfile(srcfile, destpath);
  }
}
