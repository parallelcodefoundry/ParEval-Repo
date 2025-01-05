cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
add_executable(microXOR src/main.cpp src/microXOR.cpp)

# Include the Kokkos headers
target_include_directories(microXOR PUBLIC include)

# Link the Kokkos library
target_link_libraries(microXOR Kokkos::KokkosCore)

# Set the Kokkos backend
if(NOT DEFINED KOKKOS_BACKEND)
  message(FATAL_ERROR "KOKKOS_BACKEND must be defined")
endif()

# Set the C++ compiler
if(NOT DEFINED CMAKE_CXX_COMPILER)
  message(FATAL_ERROR "CMAKE_CXX_COMPILER must be defined")
endif()