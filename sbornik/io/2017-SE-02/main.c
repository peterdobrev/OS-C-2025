#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void print(int fd, bool number) {
    char buf[4096];
    int bytes;
    int line_num = 1;
    bool start_of_line = true;
    
    while((bytes = read(fd, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < bytes; i++) {
            if(number && start_of_line) {
                char line_str[20];
                int len = snprintf(line_str, sizeof(line_str), "%6d\t", line_num);
                if(write(1, line_str, len) < 0) err(1, "Couldn't write");
                line_num++;
                start_of_line = false;
            }
            
            if(write(1, &buf[i], 1) < 0) err(1, "Couldn't write");
            
            if(buf[i] == '\n') {
                start_of_line = true;
            }
        }
    }
    if(bytes < 0) {
        err(1, "Couldn't read");
    }
}

void printFile(const char* filename, bool number) {
    int fd = open(filename, O_RDONLY);
    if(fd < 0) err(1, "Couldn't open");
    print(fd, number);
    close(fd);
}

int main(int argc, char* argv[]) {
    bool number = false;
    int argOffset = 1;
    
    if(argc > 1 && strcmp(argv[1], "-n") == 0) {
        number = true;
        argOffset = 2;
    }
    
    if(argOffset >= argc) {
        print(0, number);
    } else {
        for(int i = argOffset; i < argc; i++) {
            if(strcmp(argv[i], "-") == 0) {
                print(0, number);  
            } else {
                printFile(argv[i], number);
            }
        }
    }
    
    return 0;
}
