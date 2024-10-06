CXX = g++
OMP_FLAGS = -fopenmp -Xopenmp-target -march=native
CXXFLAGS = -O3 -std=c++17 $(OMP_FLAGS)
LDFLAGS = -fopenmp

SRC = src
INC = include
OBJ = obj
CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean