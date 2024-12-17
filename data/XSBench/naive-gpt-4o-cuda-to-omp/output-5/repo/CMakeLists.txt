cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(SOURCE Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

add_executable(XSBench ${SOURCE})

# Enable OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    target_link_libraries(XSBench PUBLIC OpenMP::OpenMP_C)
endif()

# Add OpenMP offloading flags
set_target_properties(XSBench PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

# Add OpenMP offloading flags for the target architecture
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(XSBench PRIVATE -fopenmp -fopenmp-targets=nvptx-none)
    target_link_options(XSBench PRIVATE -fopenmp -fopenmp-targets=nvptx-none)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(XSBench PRIVATE -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda)
    target_link_options(XSBench PRIVATE -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda)
endif()

install(TARGETS XSBench DESTINATION bin)