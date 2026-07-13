cmake_minimum_required(VERSION 3.10)

# Project name
project(DataDrivenEngine VERSION 1.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
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
include_directories(include/Core)
include_directories(include/Engine)
include_directories(external)

# 2. Gather all source files recursively from the src folder
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 3. Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Link Raylib
# This assumes Raylib is installed on your system.
# If you built Raylib manually, you may need to use target_link_libraries 
# with the path to the library file.
find_package(raylib REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

#  # On Linux, you often need these extra system libraries to support Raylib
#  if(UNIX AND NOT APPLE)
#      target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl rt X11)
#  endif()
