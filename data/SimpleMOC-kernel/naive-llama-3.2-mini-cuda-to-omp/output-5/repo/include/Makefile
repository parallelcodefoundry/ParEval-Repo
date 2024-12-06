include/Makefile
SM_VERSION := $(shell echo "80" | tr 'a-z' 'A-Z')

all: sources_h sources_d table

sources_h: sources_h.c sources.h
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -o $@ $^

sources_d: sources_d.c table.h
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -o $@ $^

table: buildExponentialTable.c table.h
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -o $@ $^

buildExponentialTable: buildExponentialTable.o
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -o $@ $^

sources_h.o: sources_h.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

sources_d.o: sources_d.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

buildExponentialTable.o: buildExponentialTable.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

sources_h.d: sources_d.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

sources_d.d: buildExponentialTable.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

%.o: %.c
	$(CXX) -std=c++11 -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include/ -O3 -fopenmp -c $^ -o $@

clean:
	rm -f $( sources_h.o ) $( sources_d.o ) $( buildExponentialTable.o ) $( table )