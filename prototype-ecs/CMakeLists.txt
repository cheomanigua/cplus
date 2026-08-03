cmake_minimum_required(VERSION 3.16)
project(PrototypeEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Raylib
find_package(raylib REQUIRED)

# Collect source files for the empty shell
file(GLOB_RECURSE SOURCES 
    app/*.cpp
    core/*.cpp
    systems/*.cpp   # Add this
)

add_executable(prototype ${SOURCES})

# Target include directories for future modules
target_include_directories(prototype PRIVATE
    app/src
    core/include
    components/include
    systems/include # Add this
)

target_link_libraries(prototype PRIVATE raylib)
