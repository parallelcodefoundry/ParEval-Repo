cmake_minimum_required(VERSION 3.10)
project(XSBench)

# Enable OpenMP
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Target for main executable
add_executable(${PROJECT_NAME}
  XSBench.cpp
  # Add other source files as needed
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c++11
  -O3
  -Wall
  -Wextra
  -fopenmp
)

# Set the executable to be compiled on a GPU (optional)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -fopencl -ftarget-device=nvidia")