// В условието пише да се ползва pipe, но нямам иедя къде се има предвид

#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <pwd.h>
#include <sys/wait.h>

void printMessage(int fd, const char* message) {
  int written = write(fd, message, strlen(message));
  if(written < 0) err(1, "Couldn't write");
  if(written != strlen(message)) errx(1, "Couldn't write all");
}

void logInfo(const char* time_buffer, const char* username, const char* log_command) {
  int fd = open("foo.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fd < 0) err(1, "Couldn't open");
  printMessage(fd, time_buffer);
  printMessage(fd, " ");
  printMessage(fd, username);
  printMessage(fd, " ");
  printMessage(fd, log_command);
  printMessage(fd, "\n");

  close(fd);
}

void executeCommand(const char* command) {
  pid_t pid = fork();
  if(pid < 0) err(1, "Couldn't fork");

  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", command, NULL);
    err(1, "Couldn't exec");
  }
}

int main(int argc, char* argv[]) {
  struct timeval tp;
  if(gettimeofday(&tp, NULL) < 0) err(1, "Couldn't get time of day");
  
  struct tm* tm_info = localtime(&tp.tv_sec);
  if(tm_info == NULL) err(1, "Couldn't get tm");

  char time_buffer[64];
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  char log_command[1024];
  log_command[0] = '\0';
  for(int i = 0; i < argc; i++) {
    strcat(log_command, argv[i]);
    if(i != argc - 1) {
      strcat(log_command, " ");
    }
  }
  uid_t uid = getuid();
  struct passwd* passwd_info = getpwuid(uid);
  if(passwd_info == NULL) err(1, "Couldn't getpwuid");
 
  logInfo(time_buffer, passwd_info->pw_name, log_command);

  char lock_command[64];
  lock_command[0] = '\0';
  strcat(lock_command, "echo passwd -l ");
  strcat(lock_command, passwd_info->pw_name);
  executeCommand(lock_command);

  char kill_command[64];
  kill_command[0] = '\0';
  strcat(kill_command, "echo pkill -u ");
  char uidbuf[32];
  snprintf(uidbuf, sizeof(uidbuf), "%d", passwd_info->pw_uid);
  strcat(kill_command, uidbuf);
  executeCommand(kill_command);

  for(int i = 0; i < 2; i++) {
    int status;
    wait(&status);
    if(!WIFEXITED(status)) {
      err(1, "Child exited with error status");
    }
  }

  return 0;
}
