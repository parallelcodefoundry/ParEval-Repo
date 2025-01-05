cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler and other flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3 -ffast-math") # Add other optimization flags as needed

# Set Kokkos backend (CUDA in this example)
if(NOT CMAKE_CXX_COMPILER)
  message(FATAL_ERROR "CMAKE_CXX_COMPILER is not set. Please set it.")
endif

if(NOT KOKKOS_BACKEND)
  message(FATAL_ERROR "KOKKOS_BACKEND is not set. Please set it (e.g., -DKOKKOS_BACKEND=CUDA).")
endif

# List source files
set(SOURCES
    main.cpp
    kernel.cpp
    init.cpp
    io.cpp
)

# Add header file location
include_directories(include)

# Add executable
add_executable(SimpleMOC-kernel ${SOURCES})

# Link Kokkos libraries
target_link_libraries(SimpleMOC-kernel Kokkos::Kokkos)

#Add CUDA compilation flags if needed.
if ("${KOKKOS_BACKEND}" STREQUAL "CUDA")
  target_link_libraries(SimpleMOC-kernel CUDA::cuda)
  target_compile_options(SimpleMOC-kernel PRIVATE -arch sm_$ENV{SM_VERSION})
  target_compile_definitions(SimpleMOC-kernel PRIVATE KOKKOS_ENABLE_CUDA)
endif

# Optionally add debug flags
if(DEBUG)
  target_compile_options(SimpleMOC-kernel PRIVATE -g)
endif

# Optionally add profiling flags (e.g., for gprof)
if(PROFILE)
  target_compile_options(SimpleMOC-kernel PRIVATE -pg)
endif

# Optionally include exponential table
if(TABLE)
  target_compile_definitions(SimpleMOC-kernel PRIVATE TABLE)
endif
