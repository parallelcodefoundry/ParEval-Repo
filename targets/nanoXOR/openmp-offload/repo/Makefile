# Makefile for OpenMP Offload nanoXOR

CXX_COMPILER ?= clang++
SM_VERSION ?= sm_80
CXXFLAGS ?= -std=c++17 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=$(SM_VERSION)
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
