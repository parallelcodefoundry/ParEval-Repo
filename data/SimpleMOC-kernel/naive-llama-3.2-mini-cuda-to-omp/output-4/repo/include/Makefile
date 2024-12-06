CC=g++
CFLAGS=-L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/
LDFLAGS=$(CFLAGS)

SM_VERSION=80

EXE=${SM_VERSION}_offload
SRCS=include/main.c include/exponential_table.c include/initialize_device_sources.c include/build_exponential_table.c include/initialize_device_sources.h include/cuda_check_error.c
OBJS=${SRCS:.c=.o}

$(EXE): $(OBJS)
	$(CC) -std=c++11 -O3 -march=nvidia --openmp-offload-targets=device $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -std=c++11 -O3 -march=nvidia --openmp-offload-targets=device -c $<

%.h: %.cpp
	$(CC) -std=c++11 -O3 -march=nvidia --openmp-offload-targets=device -c $<

include/main.c:
	echo "export CXXFLAGS=-std=c++11" >> /etc/profile.d/nvidia_gcc.sh

include/exponential_table.h:
	echo "float maxVal = 10.0; float dx;" > include/exponential_table.h
	echo "// compute number of arry values" >> include/exponential_table.h
	echo "//int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );" >> include/exponential_table.h
	echo "// compute spacing" >> include/exponential_table.h
	echo "//float dx = maxVal / (float) N;" >> include/exponential_table.h

include/initialize_device_sources.c:
	@mkdir -p include/device
	touch include/device/initialize_device_sources.cpp

include/cuda_check_error.c:
	@mkdir -p include/device
	touch include/device/cuda_check_error.cpp

clean:
	rm -f $(EXE) ${OBJS}