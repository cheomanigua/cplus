cmake_minimum_required(VERSION 3.16)
project(sfml_test LANGUAGES CXX)

find_package(SFML 3 COMPONENTS Graphics REQUIRED)

add_executable(sfml_test main.cpp)
target_link_libraries(sfml_test PRIVATE SFML::Graphics)
