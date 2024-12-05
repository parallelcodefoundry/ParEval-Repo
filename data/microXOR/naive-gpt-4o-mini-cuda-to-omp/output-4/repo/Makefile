# Makefile for OpenMP Offload microXOR

CC ?= g++
SM_VERSION ?= 80
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -fopenmp -O3 -std=c++17 -I$(INC)
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g
endif
LDFLAGS ?= -L/usr/local/cuda/lib64 -lcudart

CXXFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CXXFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CC) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean