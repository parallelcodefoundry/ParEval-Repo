# Makefile for OpenMP Threads nanoXOR

CXX_COMPILER ?= clang++
CXXFLAGS ?= -std=c++17 -fopenmp
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g
endif
LDFLAGS ?= -lm

nanoXOR.exe: nanoXOR.cpp
	$(CXX_COMPILER) $(CXXFLAGS) nanoXOR.cpp -o nanoXOR.exe $(LDFLAGS)

clean:
	rm -f nanoXOR.exe

.PHONY: clean
