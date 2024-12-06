# Use OpenMP-offload execution model
CXXFLAGS=$(shell grep -o '.*-O3' CXXFLAGS) -fopenmp

# Compiler flags for GPU
GPUCXXFLAGS=$(shell grep -o '.*-L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/' CXXFLAGS)
GPUCXXFLAGS+=$(shell grep -o '-l/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/' GPUCXXFLAGS)

# Link against CUDA library
GPULIBS=-lcuda

# Makefile entry point
all: buildExponentialTable cudaMakefile translate

buildExponentialTable:
    $(CXX) -o $@ $^ $(CXXFLAGS) $(GPUCXXFLAGS) $(GPULIBS)

cudaMakefile:
    # Define SM version and compute capability
    SM_VERSION=80
    ComputeCapability = "SM_$(SM_VERSION)"

    # Create a kernel for table build
    template=$(cat <<EOF
__global__ void tableBuildKernel(int* values, float dx) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < $(N)) {
        float exponential = exp(-$(n)*$dx);
        values[2*$(idx)] = -exponential;
        values[2*$(idx)+1] = 1 + ($(n)*$dx-$($(N))*$dx) * exponential;
    }
}
EOF
)
    echo "$template" > tableBuildKernel.cu

translate:
    # Define table build function
    template=$(cat <<EOF
__host__ __device__ float exp(float x) {
    // Approximate Taylor series expansion of e^(-x)
    float result = 1.0;
    for (int n = 0; n < $(N); ++n) {
        result += (float)-$(n)*$dx * $result / (1.0 + $(n)*$dx);
    }
    return result;
}

__host__ void buildExponentialTable() {
    float dx = $($(maxVal) - $(dx));
    int* values = (int*)malloc($($(N))*sizeof(int));
    for (int n = 0; n < $(N); ++n) {
        values[2*n] = (int)(-$(exp(-$n*$dx)));
        values[2*n+1] = (int)(1.0 + ($($(n)*$dx)-$($(N))*$dx) * $(exp(-$n*$dx)));
    }
    free(values);
}
EOF
)
    echo "$template" > buildExponentialTable.c

clean:
    rm -f tableBuildKernel.cu buildExponentialTable.c buildExponentialTable.o buildExponentialTable.cu