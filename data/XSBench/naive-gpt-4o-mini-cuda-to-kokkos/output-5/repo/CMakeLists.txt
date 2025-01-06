cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Set Kokkos as a required package
find_package(Kokkos REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Specify the source files
set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)

# Create an executable target
add_executable(XSBench ${SOURCE})

# Link Kokkos to the executable
target_link_libraries(XSBench Kokkos::kokkos)

# Specify the Kokkos options
target_compile_definitions(XSBench PRIVATE KOKKOS_ENABLE_CUDA)
target_compile_options(XSBench PRIVATE ${Kokkos_CXX_FLAGS})

# Set the C++ standard
set_property(TARGET XSBench PROPERTY CXX_STANDARD 14)
set_property(TARGET XSBench PROPERTY CXX_STANDARD_REQUIRED ON)

# Optionally, add include directories if needed
target_include_directories(XSBench PRIVATE ${Kokkos_INCLUDE_DIRS})

# Install target
install(TARGETS XSBench DESTINATION bin)