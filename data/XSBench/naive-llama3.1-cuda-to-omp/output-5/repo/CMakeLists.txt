cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES CXX OFFLOAD
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Define OpenMP flags
set(CMAKE_OPENMP_FLAGS "-fopenmp")
set(CMAKE_OFFLOAD_FLAGS "-fopenacc")

# Add source files
add_executable(XSBench Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

# Specify include directories for OpenMP offload
target_include_directories(XSBench PRIVATE ${OPENMP_INCLUDE_DIR})

# Link against OpenMP libraries
target_link_libraries(XSBench PUBLIC ${OPENMP_LIBRARIES} ${CUDART_LIBRARY})

install(TARGETS XSBench DESTINATION bin)