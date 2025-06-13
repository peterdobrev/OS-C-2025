//
#include <stdbool.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int min(int a, int b) {
  if(a <= b) return a;
  return b;
}

int max(int a, int b) {
  if(a >= b) return a;
  return b;
}

bool isDigit(char a) {
  return a >= '0' && a <= '9';
}

int toDigit(char a) {
  if(!isDigit(a)) errx(1, "Wrong format");
  return a - '0';
}

void handleDashSep(const char* str, int* first, int* second) {
  if(strlen(str) != 3 || str[1] != '-' || !isDigit(str[0]) || !isDigit(str[2])) {
    errx(1, "wrong format");
  }
  *first = toDigit(str[0]);
  *second = toDigit(str[2]);
}

void handleSingleNum(const char* str, int* arg) {
  if(strlen(str) != 1 || !isDigit(str[0])) errx(1, "Wrong format");
  *arg = toDigit(str[0]);
}

void handleNumberArg(const char* str, int* first, int* second) {
  if(strlen(str) == 1) {
    *second = -1;
    handleSingleNum(str, first);
  } else if(strlen(str) == 3) {
    handleDashSep(str, first, second);
  } else {
    errx(1, "Wrong format");
  }
}

bool getline(int fd, char buf[], int size, char sep) {
  char c;
  int bytes;
  int ind = 0;
  while((bytes = read(fd, &c, 1)) > 0 && ind < size - 1) {
    if(c == sep) {
      // not sure whether to add to buf
      break;
    }
    buf[ind++] = c;
  }
  buf[ind] = '\0';
  if(bytes == 0) return 0; // file finished
  if(bytes < 0) err(1, "Couldn't read");
  return 1;
}

void handleC(char* argv[]) {
  int first, second;
  handleNumberArg(argv[2], &first, &second);
  char buf[1024];
  while(1) {
    bool shouldContinue = getline(0, buf, sizeof(buf), '\n');
    
    if(second == -1) {
      if((int) strlen(buf) > first - 1)
        if(write(1, &buf[first - 1], 1) < 0) err(1, "Couldn't write");

      if(write(1, "\n", 1) < 0) err(1, "Couldn't write");
    } else {
      int len = second - first + 1;
      int maxLen = max(strlen(buf) - first + 1, 0);
      len = min(len, maxLen);
      if(len != 0)
        if(write(1, &buf[first - 1], len) < 0) err(1, "Couldn't write");

      if(write(1, "\n", 1) < 0) err(1, "Couldn't write");
    }

    if(!shouldContinue) break;
  }
}

bool skipToSep(const char* from, int* ind, int len, char sep) {
  int i = 0;
  int x = 0;
  while(*from && i < len) {
    if(*from == sep) i++;
    x++;
    from++;
  }
  *ind = x;

  if(*from == '\0') return false;
  return true;
}

void readToSep(const char* from, char buf[], int size, char sep) {
  if(from == NULL) errx(1, "Passed nullptr as from");

  int ind = 0;
  while(*from && ind < size - 1) {
    if(*from == sep) {
      break;
    }
    buf[ind++] = *from;
    from++;
  }
  buf[ind] = '\0';
}

bool getIthCol(const char* from, int i, char buf[], int size, char sep) {
  int ind;
  if(!skipToSep(from, &ind, i - 1, sep)) {
    buf[0] = '\0';
    return false;
  }

  readToSep(&from[ind], buf, size, sep);
  return true;
}

void handleD(char* argv[]) {
  int first, second;
  handleNumberArg(argv[4], &first, &second);

  char sep = argv[2][0];

  char buf[1024];
  while(1) {
    bool shouldContinue = getline(0, buf, sizeof(buf), '\n');
    
    if(second == -1) {
      char ithCol[1024];
      getIthCol(buf, first, ithCol, sizeof(ithCol), sep);
      if(write(1, ithCol, strlen(ithCol)) < 0) err(1, "Couldn't write");

      if(write(1, "\n", 1) < 0) err(1, "Couldn't write");
    }
    else {
      for(int i = first; i <= second; i++) {
        char ithCol[1024];
        bool res = getIthCol(buf, i, ithCol, sizeof(ithCol), sep);
        if(write(1, ithCol, strlen(ithCol)) < 0) err(1, "Couldn't write");
        
        if(i != second && res) {
          if(write(1, &sep, 1) < 0) err(1, "Couldn't write");
        }
      }
      if(write(1, "\n", 1) < 0) err(1, "Couldn't write");
    }

    if(!shouldContinue) break;
  }
}

int main(int argc, char* argv[]) {
  if(argc <= 2) errx(1, "bad args");
  else if(argc == 3 && strcmp(argv[1], "-c") == 0) handleC(argv);
  else if(argc == 5 && strcmp(argv[1], "-d") == 0 && strcmp(argv[3], "-f") == 0) { 
    handleD(argv);
  }
  else errx(1, "Wrong format");
  return 0;
}
