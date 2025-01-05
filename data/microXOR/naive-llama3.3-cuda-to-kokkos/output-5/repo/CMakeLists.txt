cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(microXOR src/main.cpp src/microXOR.cpp)

# Link the Kokkos library
target_link_libraries(microXOR Kokkos::KokkosCore)

# Set the include directories
target_include_directories(microXOR PUBLIC include)

# Set the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Add a custom target to run the executable
add_custom_target(run
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/microXOR 1024 32
  DEPENDS microXOR
)

# Add a custom target to clean the build directory
add_custom_target(clean
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}
)