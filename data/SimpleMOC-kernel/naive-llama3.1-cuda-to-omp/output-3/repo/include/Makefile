CC = clang
CXX = clang++
CFLAGS = -fopenmp-targets=nvptx64-nvidia-cuda -D__CUDA_ARCH__=800
CXXFLAGS = $(CFLAGS) -std=c++11

all: program

program: main.o init_source_arrays.o translate_device_sources.o build_exponential_table.o __cudaCheckError.o
    $(CXX) $^ -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/ -lcublas -lcudart -o program

main.o: main.c
    $(CC) $(CFLAGS) -c $<

init_source_arrays.o: init_source_arrays.c
    $(CC) $(CFLAGS) -c $<

translate_device_sources.o: translate_device_sources.c
    $(CC) $(CFLAGS) -c $<

build_exponential_table.o: build_exponential_table.c
    $(CC) $(CFLAGS) -c $<

__cudaCheckError.o: __cudaCheckError.c
    $(CC) $(CFLAGS) -c $<

clean:
    rm -f *.o program