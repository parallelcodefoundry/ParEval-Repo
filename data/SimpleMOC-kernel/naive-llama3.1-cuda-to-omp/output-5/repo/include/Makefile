# Define variables for compilation
SM_VERSION = 80
CFLAGS = -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/
CXX = clang

# Compiler flags for OpenMP and offload
OMP_CFLAGS = -fopenmp -fPIC
OMP_OFFLOAD_CFLAGS = -fopenacc
CFLAGS += $(OMP_CFLAGS) $(OMP_OFFLOAD_CFLAGS)

# Specify the NVIDIA library paths
LIBPATH = /opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

# Specify the compiler and linker flags for offload
NVCC = nvcc
NVCCFLAGS = -gencode=arch=compute_$(SM_VERSION),code=sm_$(SM_VERSION)
LDFLAGS = -lcudart

# Specify the source files to compile
SOURCE_FILES = init.c main.c source_array.c table.c cuda_check_error.c

# Define the target binary
TARGET = app

# Define the rule for compiling and linking the program
all: $(TARGET)

$(TARGET): $(SOURCE_FILES)
    $(CXX) -o $(TARGET) $^ $(CFLAGS) $(NVCCFLAGS) $(LDFLAGS) -lcudart -I/usr/local/cuda-11.4/include

# Clean rule to remove intermediate files and the target binary
clean:
    rm -f $(TARGET) *.o