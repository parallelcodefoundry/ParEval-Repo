cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Set compiler and flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math") #Add optimization flags here.
if(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -G")
endif
if(PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif

# Find Kokkos and CUDA
find_package(Kokkos REQUIRED)
find_package(CUDA REQUIRED)

# Add include directories
include_directories(include)

# Add source files
add_executable(SimpleMOC-kernel
  include/main.cpp
  include/kernel.cpp
  include/init.cpp
  include/io.cpp)

# Link Kokkos and CUDA libraries
target_link_libraries(SimpleMOC-kernel Kokkos::Kokkos CUDA::CUDA)


# Add custom compile options if needed.
#Example:  target_compile_options(SimpleMOC-kernel PRIVATE -DTABLE)  (uncomment and adjust as needed)

#Check if TABLE option is set.
if(TABLE)
  target_compile_options(SimpleMOC-kernel PRIVATE -DTABLE)
endif
