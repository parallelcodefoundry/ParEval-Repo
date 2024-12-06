include/Makefile:
    # This file does not need to be modified

# The OpenMP-Offload compiler
CC = nvcc

# NVIDIA GPU architecture
SM_VERSION = 80

# Include path
INC_PATH = include/

# Output directory for compiled files
OUT_DIR = bin/

# Source and object files
SRC = $(wildcard *.c)
OBJ = $( SRC:.c=.o)

# Linker flags
LDFLAGS = -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

# OpenMP-Offload compiler flags
OMPFFLAGS = --openmp-offload

# Rule to compile source files into object files
$(OBJ): $(SRC)
    @echo "Compiling $@"
    $(CC) $(OMPFFLAGS) -c $(INC_PATH)$^ -o $@

# Rule to link object files into an executable
all: $(OUT_DIR)/program

$(OUT_DIR)/program: $(OBJ)
    @echo "Linking"
    $(CC) $(LDFLAGS) $(OMPFFLAGS) -o $@ $^

# Rule to clean up compiled files
clean:
    rm -f $(OBJ) $(OUT_DIR)/program