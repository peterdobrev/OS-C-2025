#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

char* words[] = {"tic ", "tac ", "toe\n"};

int printBinaryNumber(int fd, uint8_t num) {
    int written = write(fd, &num, sizeof(num));
    if(written < 0) err(1, "Couldn't write");
    if(written != sizeof(num)) errx(1, "Couldn't write");
    return written;
}

int printMessage(int fd, const char* message) {
    int written = write(fd, message, strlen(message));
    if(written < 0) err(1, "Couldn't write");
    if(written != strlen(message)) errx(1, "Couldn't write");
    return written;
}

void closePipes(int pipes[7][2], int process_id, int nc) {
    int read_pipe = (process_id - 1 + nc) % nc;
    int write_pipe = process_id % nc;
    
    for(int i = 0; i < nc; i++) {
        if(i == read_pipe) {
            close(pipes[i][1]); 
        } else if(i == write_pipe) {
            close(pipes[i][0]); 
        } else {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
}

void handleProcess(int process_id, int pipes[7][2], int wc, int nc) {
    pid_t pid = fork();
    if(pid < 0) err(1, "Couldn't fork");
    
    if(pid == 0) {
        // child process
        closePipes(pipes, process_id, nc);
        
        int read_pipe = (process_id - 1 + nc) % nc;
        int write_pipe = process_id % nc;
        
        uint8_t wordInd;
        while(1) {
            int bytes = read(pipes[read_pipe][0], &wordInd, sizeof(wordInd));
            if(bytes < 0) err(1, "Couldn't read");
            if(bytes == 0) {
                exit(0);
            }
            if(bytes != sizeof(wordInd)) errx(1, "Couldn't read all");
            
            if(wordInd >= wc) {
                // Done printing, pass the signal and exit
                printBinaryNumber(pipes[write_pipe][1], wordInd);
                close(pipes[read_pipe][0]);
                close(pipes[write_pipe][1]);
                exit(0);
            } else {
                char* word = words[wordInd % 3];
                printMessage(1, word);
                wordInd++;
                printBinaryNumber(pipes[write_pipe][1], wordInd);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3) errx(1, "bad args");
    
    int nc = strtol(argv[1], NULL, 10);  
    int wc = strtol(argv[2], NULL, 10);
    
    if(nc < 1 || nc > 7) errx(1, "NC must be between 1 and 7");
    if(wc < 1 || wc > 35) errx(1, "WC must be between 1 and 35");
    
    int pipes[7][2];
    for(int i = 0; i < nc; i++) {
        if(pipe(pipes[i]) < 0) err(1, "Couldn't pipe");
    }
    
    for(int i = 1; i <= nc; i++) {
        handleProcess(i, pipes, wc, nc);
    }
    
    closePipes(pipes, 0, nc);
    
    int read_pipe = nc - 1;
    int write_pipe = 0;
    
    uint8_t wordInd = 0;
    char* word = words[wordInd % 3];
    printMessage(1, word);
    wordInd++;
    printBinaryNumber(pipes[write_pipe][1], wordInd);
 
    while(1) {
        int bytes = read(pipes[read_pipe][0], &wordInd, sizeof(wordInd));
        if(bytes < 0) err(1, "Couldn't read");
        if(bytes != sizeof(wordInd)) errx(1, "Couldn't read all");
        
        if(wordInd >= wc) {
            printBinaryNumber(pipes[write_pipe][1], wordInd);
            break;
        } else {
            char* word = words[wordInd % 3];
            printMessage(1, word);
            wordInd++;
            printBinaryNumber(pipes[write_pipe][1], wordInd);
        }
    }
    
    close(pipes[read_pipe][0]);
    close(pipes[write_pipe][1]);
    
    for(int i = 0; i < nc; i++) {
        int status;
        pid_t child_pid = wait(&status);
        if(child_pid < 0) err(1, "Wait failed");
        if(!WIFEXITED(status)) {
            errx(1, "Child %d didn't exit normally, status: %d", child_pid, status);
        }
        if(WEXITSTATUS(status) != 0) {
            errx(1, "Child %d exited with non-zero status: %d", child_pid, WEXITSTATUS(status));
        }
    }
    
    return 0;
}
