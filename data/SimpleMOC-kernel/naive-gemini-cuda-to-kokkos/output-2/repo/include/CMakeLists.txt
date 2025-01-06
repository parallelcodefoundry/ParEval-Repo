cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler and architecture flags
set(CMAKE_CXX_STANDARD 17)
if(KOKKOS_BACKEND STREQUAL "CUDA")
  set(KOKKOS_CUDA_ARCH "sm_35") # Adjust as needed
endif

# Source files
set(SOURCES
  main.cpp
  kernel.cpp
  init.cpp
  io.cpp
)

# Header file
set(HEADERS
  SimpleMOC-kernel_header.hpp
)


# Add Kokkos include directory
include_directories(${KOKKOS_INCLUDE_DIRS})

# Add executable target
add_executable(SimpleMOC-kernel ${SOURCES})
target_link_libraries(SimpleMOC-kernel ${KOKKOS_LIBRARIES} curand)

# Set Kokkos backend
if(KOKKOS_BACKEND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKOKKOS_BACKEND=${KOKKOS_BACKEND}")
  if(KOKKOS_BACKEND STREQUAL "CUDA")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch=sm_${KOKKOS_CUDA_ARCH}")
  endif
endif

# Enable debugging and optimization based on options (optional)
if(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # Debugging flags
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG") # Optimization flags
endif

if(TABLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTABLE")
endif
