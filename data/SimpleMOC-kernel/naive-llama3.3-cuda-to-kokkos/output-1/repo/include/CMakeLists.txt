cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
set(SOURCES
  kernel.cpp
  main.cpp
  init.cpp
  io.cpp
)

# Add the header files
set(HEADERS
  SimpleMOC-kernel_header.hpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link the Kokkos library
target_link_libraries(${PROJECT_NAME} Kokkos::Kokkos)

# Set the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Set the optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Set the debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")