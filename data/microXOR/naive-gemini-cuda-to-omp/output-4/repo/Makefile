# Makefile for OpenMP offloading microXOR

CXX ?= clang++
SM_VERSION ?= 80
CXXFLAGS ?= -fopenmp -foffload=-march=sm_${SM_VERSION} -Iinclude -std=c++17
LDFLAGS ?= -fopenmp -lcudart #Note: -lcudart might not be necessary depending on your OpenMP offloading setup.  It's included for potential compatibility.

ifeq ($(DEBUG), yes)
    CXXFLAGS += -g -G
else
    CXXFLAGS += -O3
endif

SRC = src
INC = include
OBJ = obj
OBJS = $(patsubst $(SRC)/%.cu,$(OBJ)/%.o,$(wildcard $(SRC)/*.cu))
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(INC)/microXOR.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean