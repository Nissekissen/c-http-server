
# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -pthread -Wall -Wextra -Iinclude

# Source and Object files (in /src)
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

# Output executable
TARGET = http_server

# Default rule: Compile the program
all: $(TARGET)

# Link all object files into the executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Compile all source files into object files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all