ifeq ($(shell which gcc),gcc)
  # C++ compiler and linker flags
  CXX=g++
  CXXFLAGS=-std=c++14 -fopenmp -O3
  LDFLAGS=-fopenmp
endif

SM_VERSION=$(shell echo $1)

# Source files
CXX_FILES=microXOR.cpp
header_files=cellsXOR.h microXOR.h

# Dependencies
DEPENDENCIES=$(addprefix $(CXX),$(foreach file,$(header_files),\( -include $(file) \)))

# Build flags for OpenMP offload
BUILD_FLAGS=-fopenmp --target=nvptx32

# Linking flags
LINK_FLAGS=$(CXXFLAGS)

all: microXOR SM_VERSION=$(SM_VERSION)

microXOR: $(DEPENDENCIES) main.o
	$(CXX) -c $(CXX_FILES) $(CXXFLAGS)
	$(CXX) $(LINK_FLAGS) -o $@ main.o

main.o: $(CXX_FILES) cellsXOR.o microXOR.o
	$(CXX) -c $(CXX_FILES) $(BUILD_FLAGS)

cellsXOR.o: cellsXOR.cpp $(header_files)
	$(CXX) -c $(CXX_FILES) $(BUILD_FLAGS)

microXOR.h: microXOR.h
	echo "/* This is a header file. Do not edit this directly */"

clean:
	rm -f $(CXX_FILES).o main.o cellsXOR.o