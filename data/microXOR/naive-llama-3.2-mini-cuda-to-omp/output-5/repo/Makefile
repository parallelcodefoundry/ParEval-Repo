# Makefile for OpenMP-offload microkernel

CC = g++
CFLAGS = -O3 -fopenmp --target=nvcc
LDLIBS = -shared -fPIC

# Source files
SRCS = microXOR.cpp cellsXOR.cpp main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Shared library target
TARGET = microXOR.so

# Additional dependencies for OpenMP-offload
SM_VERSION ?= 80
CXXFLAGS += -D __NVCC_VERSION__=$(SM_VERSION)
CXXFLAGS += -fopenmp

$(TARGET): $(OBJS) $(LDLIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)