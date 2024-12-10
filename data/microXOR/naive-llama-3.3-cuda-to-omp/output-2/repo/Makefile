# Makefile for OpenMP-Offload microXOR

SM_VERSION ?= 80
CXX_COMPILER ?= g++

# Set compiler flags based on CXX_COMPILER
ifeq ($(CXX_COMPILER),clang++)
  CXX_FLAGS = -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --offload-arch=sm_$(SM_VERSION)
else ifeq ($(CXX_COMPILER),gcc)
  CXX_FLAGS = -fopenmp -foffload-nvptx=-misa=simd -foffload=nvptx-none
endif

# Set include and library paths for OpenMP-Offload
INCLUDE_PATHS = 
LIBRARY_PATHS = 

# Source files
SRC_FILES = main.cpp microXOR.cpp

# Object files
OBJ_FILES = $(SRC_FILES:.cpp=.o)

# Header files
HDR_FILES = microXOR.hpp

# Build rules
%.o: %.cpp $(HDR_FILES)
	$(CXX_COMPILER) -c $< $(CXX_FLAGS) -o $@

microXOR-offload: $(OBJ_FILES)
	$(CXX_COMPILER) $(CXX_FLAGS) $(OBJ_FILES) -o $@ $(LIBRARY_PATHS)

# Clean rule
clean:
	rm -f *.o microXOR-offload

# Default target
all: microXOR-offload

.PHONY: clean all