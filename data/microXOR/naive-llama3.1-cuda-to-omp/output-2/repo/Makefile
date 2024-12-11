# OpenMP-offload Makefile
CXX = $(CXX_COMPILER)
SM_VERSION := $(SM_VERSION)
OFFLOAD_DEVICES = nvidia
OFFLOAD_ARCHS = $(SM_VERSION)

default: main.cpp microXOR.cpp
	$(CXX) -fopenmp -foffload=$(OFFLOAD_DEVICES),arch=$(OFFLOAD_ARCHS) $^ -o main

clean:
	rm -rf main

%.o : %.cpp
	$(CXX) -c -fopenmp -foffload=$(OFFLOAD_DEVICES),arch=$(OFFLOAD_ARCHS) $< -o $@

microXOR.o: microXOR.cpp
main.o: main.cpp

microXOR.cpp:
main.cpp: