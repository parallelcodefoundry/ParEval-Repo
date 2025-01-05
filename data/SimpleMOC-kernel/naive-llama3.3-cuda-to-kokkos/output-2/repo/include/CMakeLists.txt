cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source files
set(SOURCES
  kernel.cpp
  main.cpp
  init.cpp
  io.cpp
)

# Set the header files
set(HEADERS
  SimpleMOC-kernel_header.hpp
)

# Add the executable
add_executable(SimpleMOC-kernel ${SOURCES})

# Link the Kokkos library
target_link_libraries(SimpleMOC-kernel Kokkos::KokkosCore)

# Add include directories
target_include_directories(SimpleMOC-kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set the CUDA architecture
if (KOKKOS_ENABLE_CUDA)
  set(CUDA_ARCHITECTURE "52" CACHE STRING "CUDA architecture")
  set_property(TARGET SimpleMOC-kernel PROPERTY CUDA_ARCHITECTURE ${CUDA_ARCHITECTURE})
endif()

# Set the optimization flags
if (NOT DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Set the debug flags
if (DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -G")
endif()

# Set the profiling flags
if (PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()