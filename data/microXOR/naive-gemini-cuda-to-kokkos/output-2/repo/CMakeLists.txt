cmake_minimum_required(VERSION 3.10)
project(microXOR)

set(CMAKE_CXX_STANDARD 17)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Add include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/microXOR.cpp
)

# Add Kokkos library
target_link_libraries(microXOR Kokkos::Kokkos)


# Add executable
add_executable(microXOR ${SOURCES})

# Set compiler flags (optional)
# target_compile_options(microXOR PRIVATE -O3) #For Release
# target_compile_options(microXOR PRIVATE -g -G)  #For Debug
