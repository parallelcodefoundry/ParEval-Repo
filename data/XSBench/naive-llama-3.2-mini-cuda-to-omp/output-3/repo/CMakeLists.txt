cmake_minimum_required(VERSION 3.10)
project(XSBench)

# Add dependencies
add_dependencies(XSBench PUBLIC ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
target_compile_options(XSBench private OFFLOAD)
target_link_libraries(XSBench PRIVATE offload)

# Create a shared library for non-threaded builds
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/offload/liboffload.a)

# Create an executable for threaded builds with OpenMP Offload
add_executable(XSBench ${CMAKE_BINARY_DIR}/XSBench.o)
target_link_libraries(XSBench OFFLOAD ${CMAKE_BINARY_DIR}/liboffload.a ${PROJECT_SOURCE_FILES})

# Target files for compilation and linking
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the profiling header file to include in the build
file(GLOB RECURSE profile.h PROFILES/*.h)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES} ${CMAKE_BINARY_DIR}/profile.o)

# Create a function pointer type for profiling
typedef void (*ProfileFunction)(void);
set(CMAKE_CXX_EXTENSIONS OFF)