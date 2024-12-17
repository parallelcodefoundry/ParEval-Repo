# Makefile for OpenMP-offload microXOR

CXX ?= g++
SM_VERSION ?= 80
OFFLOAD_COMPILER ?= cl-clang++
OFFLOAD_FLAGS ?= -ccbin=nvcc --offload-architecture=${SM_VERSION}
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g
endif

CXX_COMPILER ?= clang++

INC = include
SRC = src
OBJ = obj
DEPS = $(INC)/microXOR.hpp
CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -fopenmp-targets=nvptx64-nvidia-cc${SM_VERSION} -I$(INC) $(OFFLOAD_FLAGS) $(OBJS) -o $(EXE)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX_COMPILER) $(CXXFLAGS) -fopenmp-targets=nvptx64-nvidia-cc${SM_VERSION} -I$(INC) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)