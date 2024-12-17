# Makefile for OpenMP microXOR

CXX ?= clang++
SM_VERSION ?= 80
CXXFLAGS ?= -fopenmp -std=c++17 -march=nvptx64-nvidia-cuda=${SM_VERSION}
ifeq ($(DEBUG), yes)
	CXXFLAGS += -g -G
else
	CXXFLAGS += -O3
endif
LDFLAGS ?= -lcudart

CPPFILES = src/microXOR.cpp src/main.cpp
HDRS = include/microXOR.hpp

OBJS = $(CPPFILES:src/%.cpp=obj/%.o)

EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

obj/%.o: src/%.cpp $(HDRS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean

# Define the offload directive
%.offload.cpp: %.cpp
	sed -e 's/^/openmp target nvptx64-nvidia-cuda=${SM_VERSION}:\n/' $< > $@