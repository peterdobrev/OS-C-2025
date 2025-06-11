#include <unistd.h>
#include <err.h>
#include <string.h>

void swap(char buf[], char* from, char* to) {
  int bufSize = strlen(buf);
  int fromSize = strlen(from);
  int toSize = strlen(to);
  if(fromSize != toSize) {
    errx(1, "Need to be of the same size");
  }
  
  for(int i = 0; i < fromSize; i++) {
    for(int j = 0; j < strlen(buf); j++) {
      if(buf[j] != from[i]) continue;

      buf[j] = to[i];
    }
  } 
}

void handleSwap(char* argv[]) {
  char buf[4096];
  int bytes;
  while((bytes = read(0, buf, sizeof(buf) - 1)) > 0) {
    buf[bytes]='\0';
    swap(buf, argv[1], argv[2]);

    if(write(1, buf, strlen(buf)) < 0) {
      err(1, "Couldn't write");
    }
  }

  if(bytes < 0) {
    err(1, "Couldn't read");
  }

}

void squeeze(char buf[], char* from) {
  int fromSize = strlen(from);
  
  for(int i = 0; i < fromSize; i++) {
    for(int j = 0; j < strlen(buf); j++) {
      if(buf[j] != from[i]) continue;

      int k = 0;
      for(; j+k < strlen(buf); k++) {
        if(buf[j] != buf[j+k]) {
          break;
        }
      }
      k--;
      
      // copies '\0'
      for(int x = j + 1; x+k <= strlen(buf); x++) {
        buf[x] = buf[x+k];
      }
    }
  }
}
  

void handleSqueeze(char* argv[]) {
  char buf[4096];
  int bytes;
  while((bytes = read(0, buf, sizeof(buf) - 1)) > 0) {
    buf[bytes]='\0';
    squeeze(buf, argv[2]);

    if(write(1, buf, strlen(buf)) < 0) {
      err(1, "Couldn't write");
    }
  }

  if(bytes < 0) {
    err(1, "Couldn't read");
  }
}

void del(char buf[], char* from) {
  int fromSize = strlen(from);

  for(int i = 0; i < fromSize; i++) {
    for(int j = 0; j < strlen(buf); j++) {
      if(buf[j] != from[i]) continue;

      // from j to bufSize to copy '\0'
      for(int k = j; k < strlen(buf); k++) {
        buf[k] = buf[k+1];
      }
      j--;
    }
  }
}

void handleDel(char* argv[]) {
  char buf[4096];
  int bytes;
  while((bytes = read(0, buf, sizeof(buf) - 1)) > 0) {
    buf[bytes]='\0';
    del(buf, argv[2]);

    if(write(1, buf, strlen(buf)) < 0) {
      err(1, "Couldn't write");
    }
  }

  if(bytes < 0) {
    err(1, "Couldn't read");
  }
}

int main(int argc, char* argv[]) {
  if(argc != 3 ) {
    errx(1, "bad argv");
  }
  
  if(strcmp(argv[1], "-d") == 0) {
    handleDel(argv);
  } else if(strcmp(argv[1], "-s") == 0) {
    handleSqueeze(argv);
  } else {
    handleSwap(argv);
  }

  return 0;
  
}
