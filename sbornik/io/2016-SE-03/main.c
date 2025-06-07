// 100 000 000 uint32_t = 400 000 000 bytes
// we have 256 MB Ram max which is 268 435 456 bytes
// this means that if we split our input array into two it will fit the constraint
// realistically we might need to split it a few more times to make it work in practice
// for this approach i will split the data into 2 files but you can as easily split it into 4 or 8 to make it more practical

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdint.h>

void populate_file(int start, int length, int fd_from, int fd_to) {
  lseek(fd_from, start, SEEK_SET);
  
  uint32_t cur;
  int bytes;
  while(length-- && (bytes = read(fd_from, &cur, sizeof(cur))) > 0) {
    if(write(fd_to, &cur, sizeof(cur)) < 0) {
      err(1, "Error when writing");
    }
  }
  if(bytes < 0) {
    err(1, "Error when reading");
  }
}

int cmp(const void* a, const void* b) {
  uint32_t x = *(const uint32_t*)a;
  uint32_t y = *(const uint32_t*)b;
  return (x > y) - (x < y);
}

void sort_file(const char* filename) {
  struct stat fileinfo;
  if(stat(filename, &fileinfo) != 0) {
    err(1, "Couldn't stat");
  }
  if(fileinfo.st_size % 4 != 0) {
    errx(1, "File doesn't contain only numbers");
  }
  int n = fileinfo.st_size / 4;
  uint32_t* arr = malloc(n * sizeof(uint32_t));
  if(!arr) {
    err(1, "Memory allocation failed");
  }
  
  int fd = open(filename, O_RDONLY);
  if(fd < 0) {
    err(1, "Couldn't open file");
  }
  if(read(fd, arr, n * sizeof(uint32_t)) < 0) {
    err(1, "Couldn't read file");
  }
  close(fd);
  
  qsort(arr, n, sizeof(uint32_t), cmp);
  
  fd = open(filename, O_WRONLY | O_TRUNC);
  if(fd < 0) {
    err(1, "Couldn't open file");
  }
  if(write(fd, arr, n * sizeof(uint32_t)) < 0) {
    err(1, "Couldn't write to file");
  }
  close(fd);
  free(arr);
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    errx(1, "Pass a file");
  }
  
  struct stat fileinfo;
  if(stat(argv[1], &fileinfo) != 0) {
    err(1, "Couldn't stat file");
  }
  
  int size = fileinfo.st_size;
  if(size % 4 != 0) {
    errx(1, "File doesn't contain only uint32_t numbers");
  }
  
  // separating the file in 2 parts
  int half = size / 2;
  int start1 = 0;
  int start2 = half;
  int elements_count = size / 4;
  
  // names
  char tmp_file1[] = "temp_sorting_help_file1";
  char tmp_file2[] = "temp_sorting_help_file2";
  
  int fd1 = open(tmp_file1, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, 0644);
  if(fd1 < 0) {
    err(1, "Couldn't create temp file");
  }
  int fd2 = open(tmp_file2, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, 0644);
  if(fd2 < 0) {
    err(1, "Couldn't create temp file");
  }
  
  int fd_from = open(argv[1], O_RDONLY);
  if(fd_from < 0) {
    err(1, "Couldn't open input file");
  }
  
  populate_file(start1, (elements_count + 1) / 2, fd_from, fd1);
  populate_file(start2, elements_count / 2, fd_from, fd2);
  
  close(fd1);
  close(fd2);
  close(fd_from);
  
  // Sort both temporary files
  sort_file(tmp_file1);
  sort_file(tmp_file2);
  
  // Merge the sorted files back into the original file
  int fd_to = open(argv[1], O_WRONLY | O_TRUNC);
  if(fd_to < 0) {
    err(1, "Couldn't open output file");
  }
  
  fd1 = open(tmp_file1, O_RDONLY);
  if(fd1 < 0) {
    err(1, "Couldn't open temp file 1");
  }
  fd2 = open(tmp_file2, O_RDONLY);
  if(fd2 < 0) {
    err(1, "Couldn't open temp file 2");
  }
  
  uint32_t cur1, cur2;
  int bytes1, bytes2;
  int has_data1 = 1, has_data2 = 1;
  
  // Read initial values
  bytes1 = read(fd1, &cur1, sizeof(cur1));
  bytes2 = read(fd2, &cur2, sizeof(cur2));
  
  if(bytes1 <= 0) has_data1 = 0;
  if(bytes2 <= 0) has_data2 = 0;
  
  // Merge loop
  while(has_data1 || has_data2) {
    if(!has_data1) {
      // Only file2 has data left
      if(write(fd_to, &cur2, sizeof(cur2)) < 0) {
        err(1, "Error writing to output");
      }
      bytes2 = read(fd2, &cur2, sizeof(cur2));
      if(bytes2 <= 0) has_data2 = 0;
    } else if(!has_data2) {
      // Only file1 has data left
      if(write(fd_to, &cur1, sizeof(cur1)) < 0) {
        err(1, "Error writing to output");
      }
      bytes1 = read(fd1, &cur1, sizeof(cur1));
      if(bytes1 <= 0) has_data1 = 0;
    } else {
      // Both files have data - compare and write smaller value
      if(cur1 <= cur2) {
        if(write(fd_to, &cur1, sizeof(cur1)) < 0) {
          err(1, "Error writing to output");
        }
        bytes1 = read(fd1, &cur1, sizeof(cur1));
        if(bytes1 <= 0) has_data1 = 0;
      } else {
        if(write(fd_to, &cur2, sizeof(cur2)) < 0) {
          err(1, "Error writing to output");
        }
        bytes2 = read(fd2, &cur2, sizeof(cur2));
        if(bytes2 <= 0) has_data2 = 0;
      }
    }
  }
  
  close(fd1);
  close(fd2);
  close(fd_to);
  
  // Clean up temporary files
  unlink(tmp_file1);
  unlink(tmp_file2);
  
  return 0;
}
