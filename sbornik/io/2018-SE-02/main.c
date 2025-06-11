#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>

int cmpfunc(const void* a, const void* b) {
  int arg1 = *(const int *)a;
  int arg2 = *(const int *)b;
  if(arg1 > arg2) {
    return 1;
  } else if(arg1 < arg2) {
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    errx(1, "bad args");
  }
  
  char tempname1[] = "/tmp/tmp1_XXXXXX";
  char tempname2[] = "/tmp/tmp2_XXXXXX";
  
  int tmp1_fd = mkstemp(tempname1);
  if(tmp1_fd < 0) err(1, "Couldn't make temp file");
  int tmp2_fd = mkstemp(tempname2);
  if(tmp2_fd < 0) err(1, "Couldn't make temp file");
  
  int input_fd = open(argv[1], O_RDONLY);
  if(input_fd < 0) {
    err(1, "Couldn't open input file");
  }
  int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(output_fd < 0) {
    err(1, "Couldn't open output file");
  }
  
  struct stat file_info;
  if(fstat(input_fd, &file_info) < 0) {
    err(1, "Couldn't stat file");
  }
  int size = file_info.st_size;
  if(size % 4 != 0) {
    errx(1, "File is not in correct format");
  }
  
  // get space for the ceil of size/2
  int capacity = (size / 4) * 2 * sizeof(uint32_t) + sizeof(uint32_t);
  
  uint32_t* arr = malloc(capacity);
  if(arr == NULL) err(1, "Couldn't alloc memory");
  
  // Read, sort, and write first half
  int bytes;
  if((bytes = read(input_fd, arr, capacity)) < 0) {
    err(1, "Couldn't read");
  }
  if(bytes % sizeof(uint32_t) != 0) {
    errx(1, "Tegavo");
  }
  qsort(arr, bytes / sizeof(uint32_t), sizeof(uint32_t), cmpfunc);
  
  if(write(tmp1_fd, arr, bytes) < 0) {
    err(1, "Couldn't write");
  }
  
  // Read, sort, and write second half
  if((bytes = read(input_fd, arr, capacity)) < 0) {
    err(1, "Couldn't read");
  }
  if(bytes % sizeof(uint32_t) != 0) {
    errx(1, "Tegavo");
  }
  qsort(arr, bytes / sizeof(uint32_t), sizeof(uint32_t), cmpfunc);
  
  if(write(tmp2_fd, arr, bytes) < 0) {
    err(1, "Couldn't write");
  }
  
  // Rewind temp files to beginning for reading
  if(lseek(tmp1_fd, 0, SEEK_SET) < 0) {
    err(1, "Couldn't seek temp file 1");
  }
  if(lseek(tmp2_fd, 0, SEEK_SET) < 0) {
    err(1, "Couldn't seek temp file 2");
  }
  
  // Merge phase
  int hasSpace1 = 1, hasSpace2 = 1;
  int shouldRead1 = 1, shouldRead2 = 1;
  uint32_t cur1, cur2;
  int bytes1, bytes2;
  
  while(hasSpace1 && hasSpace2) {
    if(shouldRead1) {
      bytes1 = read(tmp1_fd, &cur1, sizeof(cur1));
      if(bytes1 < 0) err(1, "Couldn't read");
      if(bytes1 == 0) hasSpace1 = 0;
    }
    if(shouldRead2) {
      bytes2 = read(tmp2_fd, &cur2, sizeof(cur2));
      if(bytes2 < 0) err(1, "Couldn't read");
      if(bytes2 == 0) hasSpace2 = 0;
    }
    
    if(hasSpace1 && hasSpace2) {
      if(cur1 <= cur2) {
        shouldRead2 = 0;
        shouldRead1 = 1;
        if(write(output_fd, &cur1, sizeof(cur1)) < 0) {
          err(1, "Couldn't write");
        }
      } else {
        shouldRead1 = 0;
        shouldRead2 = 1;
        if(write(output_fd, &cur2, sizeof(cur2)) < 0) {
          err(1, "Couldn't write");
        }
      }
    }
  }
  
  // Write remaining elements from file 1
  if(hasSpace1) {
    // Write the current element if we have one
    if(shouldRead1 == 0) {  // We have cur1 but haven't written it
      if(write(output_fd, &cur1, sizeof(cur1)) < 0) {
        err(1, "Couldn't write");
      }
    }
    // Write remaining elements
    while((bytes1 = read(tmp1_fd, &cur1, sizeof(cur1))) > 0) {
      if(write(output_fd, &cur1, sizeof(cur1)) < 0) {
        err(1, "Couldn't write");
      }
    }
    if(bytes1 < 0) {
      err(1, "Couldn't read");
    }
  }
  
  // Write remaining elements from file 2
  if(hasSpace2) {
    // Write the current element if we have one
    if(shouldRead2 == 0) {  // We have cur2 but haven't written it
      if(write(output_fd, &cur2, sizeof(cur2)) < 0) {
        err(1, "Couldn't write");
      }
    }
    // Write remaining elements
    while((bytes2 = read(tmp2_fd, &cur2, sizeof(cur2))) > 0) {
      if(write(output_fd, &cur2, sizeof(cur2)) < 0) {
        err(1, "Couldn't write");
      }
    }
    if(bytes2 < 0) {
      err(1, "Couldn't read");
    }
  }
  
  free(arr);
  close(input_fd);
  close(output_fd);
  close(tmp1_fd); 
  close(tmp2_fd);
  unlink(tempname1);
  unlink(tempname2);
  return 0;
}
