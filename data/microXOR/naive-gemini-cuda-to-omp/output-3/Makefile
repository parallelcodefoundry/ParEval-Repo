CXX = g++
CXXFLAGS ?= -O3 -fopenmp -Iinclude -std=c++17
LDFLAGS ?= -fopenmp

SRC = src
INC = include
OBJ = obj
EXE = microXOR

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp

OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean