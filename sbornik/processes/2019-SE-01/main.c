#include <string.h>
#include <time.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void printMessage(int fd, const char* message) {
  int bytes = write(fd, message, strlen(message));
  if(bytes < 0) err(1, "Couldn't write");
  if(bytes != strlen(message)) err(1, "Couldn't write all");
}

void printNumber(int fd, int number) {
  char buf[11 + 1]; // for null terminate
  int len = snprintf(buf, sizeof(buf), "%d", number);
  // validation if len is number length needed here
  buf[len] = '\0';
  printMessage(fd, buf);
}

void logData(int fd, int start, int end, int exitCode) {
  printNumber(fd, start);
  printMessage(fd, " ");
  printNumber(fd, end);
  printMessage(fd, " ");
  printNumber(fd, exitCode);
  printMessage(fd, "\n");
}

int runCommand(const char* command, const char** commandArgs, int log_fd, int* timeArg) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");
  
  int exitCode = 0;

  if(pid == 0) {
    // child
    execvp(command, (char* const*)commandArgs);
    err(1, "execvp failed");
  } else {
    int startTime = time(NULL);
    
    int status;
    wait(&status);

    int endTime = time(NULL);

    if(WIFEXITED(status)) {
      exitCode = WEXITSTATUS(status);
    } else if(WIFSIGNALED(status)) {
      exitCode = 129;
    }

    logData(log_fd, startTime, endTime, exitCode);
    *timeArg = (endTime - startTime);
  }

  return exitCode;
}

int main(int argc, char* argv[]) {
  if(argc < 3) errx(1, "Bad args");

  int log_fd = open("run.log", O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(log_fd < 0) err(1, "Couldn't open log file");

  int minTime = strtol(argv[1], NULL, 10);
  if(minTime < 0) err(1, "Couldn't convert to number");

  if(minTime < 1 || minTime > 9) errx(1, "Bad args");
 
 // copy args of commands to array
  char** commandArgs = malloc((argc - 2  + 1) * sizeof(char*));
  for(int i = 2; i < argc; i++) {
    commandArgs[i-2] = argv[i];
  }
  commandArgs[argc - 2] = NULL;

  bool terminationCondition = false;
  while(true) {
    int time;
    int code = runCommand(commandArgs[0], (const char**) commandArgs, log_fd, &time);
    if(code != 0 && time < minTime) {
      if(terminationCondition) break;

      terminationCondition = true;
    } else {
      terminationCondition = false;
    }
  }

  close(log_fd);
  free(commandArgs);

  return 0;
}
