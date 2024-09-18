# Compiler
CC = mpicc

# Directories
SRC_DIR = src
OBJ_DIR = objects
HEADER_DIR = headers

# Source files, object files, and target
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/read_write.c $(SRC_DIR)/initialize.c $(SRC_DIR)/static_evolution.c $(SRC_DIR)/ordered_evolution.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = main.x

# Compiler flags
CFLAGS = -fopenmp -I$(HEADER_DIR)

# Default target
.PHONY: all clean

all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Rule for compiling .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure the object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up object files and executable
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
