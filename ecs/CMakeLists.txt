cmake_minimum_required(VERSION 3.10)

# Project name
project(DataDrivenEngine VERSION 1.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#  # Optimization Flags for Performance
#  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#      # -O3: Enables aggressive optimization
#      # -march=native: Optimizes for your specific CPU
#      # -fopt-info-vec-optimized: Triggers reports on vectorization
#      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -fopt-info-vec-optimized")
#  endif()

# 1. Define include directories
# This allows you to include headers like #include "Engine/EntityRegistry.hpp"
include_directories(include)
include_directories(external)

# 2. Gather all source files recursively from the src folder
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 3. Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Optional: Add Raylib if you are using it (Uncomment these if needed)
# find_package(raylib REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
