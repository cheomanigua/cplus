cmake_minimum_required(VERSION 3.16)

project(game_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Dependencies
# ============================================================

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(raylib REQUIRED)

# ============================================================
# SFML executable
# ============================================================

add_executable(game_sfml
    main_sfml.cpp
)

target_link_libraries(game_sfml PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# ============================================================
# raylib executable
# ============================================================

add_executable(game_raylib
    main_raylib.cpp
)

target_link_libraries(game_raylib PRIVATE
    raylib
)
