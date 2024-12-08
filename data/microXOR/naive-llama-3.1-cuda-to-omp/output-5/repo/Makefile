# Makefile translated from CUDA to OpenMP-offload

CXX = clang++
CXXFLAGS = -fopenmp -fopenacc
LDLIBS = -lomp
SM_VERSION ?= $(shell ompi_info --version)
ifeq ($(SM_VERSION), 80)
    CXXFLAGS += -acc -Minfo=all
endif

CC = clang
CFLAGS = -fopenmp -fopenacc

# Default target is build
build: microXOR.cpp microXOR.hpp main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

main.o: main.cpp microXOR.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

microXOR.o: microXOR.cpp microXOR.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleaning target
clean:
	rm -f *.o build

# Target for building for a specific SM version
build-sm-80: CXX = clang++
build-sm-80: LDLIBS = -lomp
build-sm-80: microXOR.cpp microXOR.hpp main.cpp
	$(CXX) $(CXXFLAGS) -acc -Minfo=all $^ -o $@ $(LDLIBS)

# Target for building with a specific C++ compiler
build-cxx-compiler: CXX = clang++
build-cxx-compiler: LDLIBS =
build-cxx-compiler: microXOR.cpp microXOR.hpp main.cpp
	$(CXX) $(CXXFLAGS) $^ -o build $(LDLIBS)