# Makefile for OpenMP-Offload Code

CC = g++
CFLAGS = -O2 -fopenmp -std=c++11
LDFLAGS = -fopenmp

SM_VERSION=$(shell echo $@ | cut -d '=' -f 2)
GPUCC65 = nvcc --compiler-gpu-arch=sm_$(SM_VERSION)

main: main.o cellsXOR.o
	$(CC) $(CFLAGS) -o main main.o cellsXOR.o -lm

cellsXOR.o: cellsXOR.cpp
	$(GPUCC65) $(LDFLAGS) -c cellsXOR.cpp -o cellsXOR.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

clean:
	rm -f main cellsXOR.o main.o