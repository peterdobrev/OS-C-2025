#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

int main() {
  char file[] = "/etc/passwd";
  char newName[] = "etcpasswd";

  char cwd[1024];
  if(getcwd(cwd, sizeof(cwd)) != NULL) {
    write(1, cwd, strlen(cwd));
    write(1, "\n", 1);
  }
  else {
    err(1, "Couldn't get current working directory.");
  }

  char destDir[1024];
  int len = snprintf(destDir, sizeof(destDir), "%s/%s", cwd, newName);
  if(len >= sizeof(destDir)) {
    errx(1, "Error! Pathfile is too long!");
  }

  int readfd = open(file, O_RDONLY);
  if(readfd < 0) {
    err(1, "Error when opening %s", file);
  }

  int writefd = open(destDir, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(writefd < 0) {
    close(readfd);
    err(1, "Error when creating %s", destDir);
  }

  char c;
  int bytes;
  while((bytes = read(readfd, &c, sizeof(c))) > 0) {
    if(c == ':') {
      c = '?';
    }

    int written = write(writefd, &c, bytes);
    if(written != bytes) {
      close(readfd);
      close(writefd);
      err(1, "Couldn't write all bytes!");
    }    
  }

  if(bytes < 0) {
    close(readfd);
    close(writefd);
    err(1, "Couldn't read from file!");
  }

  close(readfd);
  close(writefd);
  return 0;
}
