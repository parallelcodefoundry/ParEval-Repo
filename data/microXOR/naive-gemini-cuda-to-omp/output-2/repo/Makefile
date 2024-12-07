# Makefile for OpenMP offloading microXOR

CXX ?= $(CXX_COMPILER)
SM_VERSION ?= $(SM_VERSION)
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -I$(INC) -std=c++17 -fPIC -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda
CXXFLAGS += -march=native
# Add -O3 for optimization or -g for debugging as needed.
ifeq ($(DEBUG), yes)
	CXXFLAGS += -g -G
else
	CXXFLAGS += -O3
endif
LDFLAGS ?= -lcudart -fopenmp

ifneq ($(CXX_COMPILER),clang++)
  # For g++, add necessary flags for OpenMP offloading with g++
  CXXFLAGS += -foffload=-march=sm_$(SM_VERSION)
endif

CPPFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
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