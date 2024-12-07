# Makefile for OpenMP offloading microXOR

CXX ?= $(CXX_COMPILER)
SM_VERSION ?= $(SM_VERSION)
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -I$(INC) -std=c++17 -fPIC -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -march=native
ifeq ($(DEBUG), yes)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif
LDFLAGS ?= -lcudart -lgomp

CPPFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CPPFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -fopenmp

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean