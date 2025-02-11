
CXX = g++
CXXFLAGS = -fopenmp -fopenmp-targets=nvptx-none -O3

SRC = repo/include/init.hpp repo/include/kernel.hpp repo/include/main.hpp

all: main

main: $(SRC)
$(CXX) $(CXXFLAGS) -o main $(SRC)

clean:
rm -f main