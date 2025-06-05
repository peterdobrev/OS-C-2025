#include <err.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if(argc != 3) {
        errx(1, "Expected two arguments");
    }
    
    int readfd = open(argv[1], O_RDONLY);
    if(readfd < 0) {
        err(1, "Error opening %s", argv[1]);
    }
    
    int writefd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(writefd < 0) {
        close(readfd);
        err(1, "Error opening %s", argv[2]);
    }
    
    char buf[16];
    int bytes;
    while((bytes = read(readfd, buf, sizeof(buf))) > 0) {
        int wrote = write(writefd, buf, bytes);  // Write only what we read
        if(wrote < 0) {
            close(readfd);
            close(writefd);
            err(1, "Error while writing to file %s", argv[2]);
        }
    }
    
    if(bytes < 0) {
        close(readfd);
        close(writefd);
        err(1, "Error when reading from file %s", argv[1]);    
    }
    
    close(readfd);
    close(writefd);
    return 0;
}
