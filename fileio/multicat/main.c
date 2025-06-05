#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <libgen.h>

int copyfile(const char* src, const char* dest) {
  int srcfd = open(src, O_RDONLY);
  if(srcfd < 0) {
    err(1, "Cannot open %s", src);
  }

  int destfd = open(dest, O_WRONLY, O_CREAT, O_TRUNC, 0644);
  if(destfd < 0) {
    close(srcfd);
    err(1, "Cannot open %s", dest);
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
    close(srcfd);
    close(destfd);
    err(1, "Error reading from %s", src);
  }
    
  close(srcfd);
  close(destfd);
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
    destpath[0] = '\0';
    if(strlen(destdir) + strlen(filename) + 2 >= sizeof(destpath)) {
      errx(1, "Pathname too long!");
    }
    strncat(destpath, destdir, sizeof(destpath) - 1);
    strncat(destpath, "/", sizeof(destpath) - strlen(destpath) - 1);
    strncat(destpath, filename, sizeof(destpath) - strlen(destpath) - 1);
    copyfile(srcfile, destpath);
  }
}
