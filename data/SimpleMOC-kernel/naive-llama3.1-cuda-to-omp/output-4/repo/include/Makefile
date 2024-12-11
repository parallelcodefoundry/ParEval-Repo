# We define a variable for our SM_VERSION (compute capability)
SM_VERSION ?= 80

# We specify the path to the NVIDIA libraries
CUDA_LIB_PATH ?= /opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

# We specify the compiler and CXX flags
CXX = clang
CXXFLAGS = -std=c++11 -fopenmp

# We compile each file separately using the NVCC compiler (which supports offloading to NVIDIA GPUs)
.cu.o:
    $(NVCC) $(CXXFLAGS) -D__CUDA_ARCH__=$(SM_VERSION) -ccbin=$(CXX) -I. $< -o $@

# We link all object files together
%.exe: %.cu.o
    $(NVCC) $(LDFLAGS) $^ -L$(CUDA_LIB_PATH) -lcudart -lnvtoolkit $(LIBS) -o $@