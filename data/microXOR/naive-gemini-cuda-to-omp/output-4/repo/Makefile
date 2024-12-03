# Makefile for OpenMP offloading microXOR

CXX ?= g++
SM_VERSION ?= 80
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -I$(INC) -std=c++17 -fopenmp -foffload=$(SM_VERSION) -foffload=-march=sm_$(SM_VERSION)
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g -G
endif
LDFLAGS ?= -fopenmp

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