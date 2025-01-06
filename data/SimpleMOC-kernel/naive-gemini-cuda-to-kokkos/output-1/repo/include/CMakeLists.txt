cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") # Or a more recent standard as needed

# Set Kokkos specific flags and include directories.  Adjust as needed for your system.
if(KOKKOS_BACKEND STREQUAL "CUDA")
  set(CMAKE_CUDA_ARCHITECTURES "sm_35") # Or adjust as appropriate for your GPU architecture
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKOKKOS_BACKEND=CUDA")
endif

# Add source files
add_executable(SimpleMOC-kernel
  main.cpp
  kernel.cu
  init.cpp
  io.cpp
)

# Link Kokkos
target_link_libraries(SimpleMOC-kernel Kokkos::Kokkos)


# Handle CUDA specific linking if needed.
if(KOKKOS_BACKEND STREQUAL "CUDA")
  target_link_libraries(SimpleMOC-kernel ${CUDA_LIBRARIES})
  target_compile_options(SimpleMOC-kernel PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_${CMAKE_CUDA_ARCHITECTURES}>)
endif

# Add options for optimization, debug, and profiling.
if(OPTIMIZE STREQUAL "yes")
  target_compile_options(SimpleMOC-kernel PRIVATE -O3 -ffast-math)
endif

if(DEBUG STREQUAL "yes")
  target_compile_options(SimpleMOC-kernel PRIVATE -g)
endif

if(PROFILE STREQUAL "yes")
  target_compile_options(SimpleMOC-kernel PRIVATE -pg)
endif


if(TABLE STREQUAL "yes")
  target_compile_options(SimpleMOC-kernel PRIVATE -DTABLE)
endif

