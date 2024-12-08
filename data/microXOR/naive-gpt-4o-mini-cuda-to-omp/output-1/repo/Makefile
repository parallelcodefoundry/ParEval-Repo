# Makefile for OpenMP Offload microXOR

CXX ?= $(CXX_COMPILER)
SM_VERSION ?= 80
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -fopenmp -fopenmp-targets=nvptx-none -I$(INC) -std=c++17
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g
endif
LDFLAGS ?= -lomp

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