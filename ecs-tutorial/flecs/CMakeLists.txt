cmake_minimum_required(VERSION 3.16)
project(PrototypeEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib REQUIRED)

# Flecs distribution
set(FLECS_DIR ${CMAKE_SOURCE_DIR}/flecs)

# Build Flecs from its C source
add_library(flecs STATIC
    ${FLECS_DIR}/flecs.c
)

target_include_directories(flecs PUBLIC
    ${FLECS_DIR}
)

# Automatically find application source files
file(GLOB_RECURSE SOURCES 
    app/*.cpp
    systems/*.cpp
)

add_executable(tutorial ${SOURCES})

# Add application include paths
target_include_directories(tutorial PRIVATE
    app/src
    components/include
    systems/include
)

target_link_libraries(tutorial PRIVATE raylib flecs)

