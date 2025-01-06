cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Add source files
set(SOURCES
  main.cpp
  kernel.cpp
  init.cpp
  io.cpp
)

# Add header file
set(HEADERS SimpleMOC-kernel_header.hpp)

# Add Kokkos-specific compilation flags
if(KOKKOS_BACKEND STREQUAL "CUDA")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKOKKOS_BACKEND=CUDA")
endif

# Create executable
add_executable(SimpleMOC-kernel ${SOURCES} ${HEADERS})

# Link against Kokkos libraries
target_link_libraries(SimpleMOC-kernel Kokkos::Kokkos)

# Add optional flags for debugging, optimization and profiling
if(DEBUG STREQUAL "yes")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif

if(OPTIMIZE STREQUAL "yes")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
endif

if(PROFILE STREQUAL "yes")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif

# Add TABLE flag if needed
if(TABLE STREQUAL "yes")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTABLE")
endif
