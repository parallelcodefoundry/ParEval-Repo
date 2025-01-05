cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source and header files
set(SOURCES src/microXOR.cpp src/main.cpp)
set(HEADERS include/microXOR.hpp)

# Add the executable
add_executable(microXOR ${SOURCES} ${HEADERS})

# Link the Kokkos library
target_link_libraries(microXOR Kokkos::KokkosCore)

# Set the include directories
target_include_directories(microXOR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set the C++ compiler
if(NOT CMAKE_CXX_COMPILER)
  message(FATAL_ERROR "CMAKE_CXX_COMPILER is not set. Please set it using -DCMAKE_CXX_COMPILER.")
endif()

# Set the Kokkos backend
if(NOT KOKKOS_BACKEND)
  message(FATAL_ERROR "KOKKOS_BACKEND is not set. Please set it using -DKOKKOS_BACKEND.")
endif()