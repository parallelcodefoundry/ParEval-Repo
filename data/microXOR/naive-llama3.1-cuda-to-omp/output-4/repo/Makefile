CXX = $(CXX_COMPILER)
CXXFLAGS = -fopenmp-targets=nvptx64-nvidia-cuda -O3
LIBS = -L/usr/lib/nvidia-opencl-dev -lcudadevrt -lcusolver -lnvrtc

all: microXOR

microXOR: microXOR.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -I. -gencode arch=compute_80,code="sm_80"

clean:
	rm -f microXOR