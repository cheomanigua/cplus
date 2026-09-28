cmake_minimum_required(VERSION 3.16)
project(PrototypeEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib REQUIRED)

# Automatically find application source files
file(GLOB_RECURSE SOURCES 
    app/*.cpp
    systems/*.cpp
)

add_executable(tutorial ${SOURCES})

# Add application include paths
target_include_directories(tutorial PRIVATE
    app/src
    core/include
    components/include
    systems/include
)

target_link_libraries(tutorial PRIVATE raylib)

