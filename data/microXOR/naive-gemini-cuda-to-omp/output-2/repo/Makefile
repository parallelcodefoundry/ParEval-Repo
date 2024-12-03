# Makefile for OpenMP offloading microXOR

CXX ?= g++
SM_VERSION ?= 80
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -I$(INC) -std=c++17 -fPIC -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda:$(SM_VERSION)
LDFLAGS ?= -lcudart -fopenmp

# We rename the source files to .cpp for OpenMP offloading
CPPFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.h
OBJS = $(CPPFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
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