cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source and header files
set(SOURCES
  kernel.cpp
  main.cpp
  init.cpp
  io.cpp
)

set(HEADERS
  SimpleMOC-kernel_header.hpp
)

# Add the executable
add_executable(SimpleMOC-kernel ${SOURCES})

# Link the Kokkos library
target_link_libraries(SimpleMOC-kernel Kokkos::KokkosCore)

# Set the include directories
target_include_directories(SimpleMOC-kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set the compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Set the debug flags
option(DEBUG "Enable debug mode" OFF)
if(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

# Set the optimization flags
option(OPTIMIZE "Enable optimization" ON)
if(OPTIMIZE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Set the profiling flags
option(PROFILE "Enable profiling" OFF)
if(PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()

# Set the table flags
option(TABLE "Enable exponential table" OFF)
if(TABLE)
  add_definitions(-DTABLE)
endif()