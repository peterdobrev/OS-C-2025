# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Target executable name
TARGET = foo.exe

# Source files
SOURCES = foo.c
# Or for multiple files: SOURCES = main.c utils.c math.c

# Object files (automatically generated from sources)
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Mark these as phony targets
.PHONY: all clean rebuild
