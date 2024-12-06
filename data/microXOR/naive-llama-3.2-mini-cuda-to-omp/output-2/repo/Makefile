# Makefile for OpenMP-Offload Code

# Define compiler flags
CXXFLAGS = -O3 -fopenmp -std=c++14

# Define CUDA device and compiler flag
SM_VERSION ?= $(shell echo "$*" | cut -d '=' -f 2)

ifneq ($($SM_VERSION),)
    CXXFLAGS += -D__CUDA_NO_HALF_EXECUTION__ -gencode arch=sm_$(SM_VERSION) -gencode target=sm_$(SM_VERSION) -I/usr/local/cuda/include
    LDFLAGS += -L/usr/local/cuda/lib64/ -linkcuda
endif

# Source files
SOURCES = $(wildcard *.cpp)

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable file
EXECUTABLE = cellsXOR

# Rule to build object files
$(OBJECTS): $(SOURCES)
    $(CXX) $(CXXFLAGS) -c $^ -o $@

# Rule to build executable
$(EXECUTABLE): $(OBJECTS)
    $(CXX) $(CXXFLAGS) -fopenmp $^ -o $%

# Clean rule
clean:
    rm -f $(OBJECTS) $(EXECUTABLE)

# Rule for CUDA device compilation
%.cu: %.o
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build the final executable
build: $(EXECUTABLE)