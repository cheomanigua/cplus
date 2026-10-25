#!/bin/bash
set -e

g++ -std=c++23 \
    $(pkg-config --cflags sdl3 sdl3-image raylib) \
    -c simplequery.cpp -o main.o

g++ main.o flecs.o \
    $(pkg-config --libs sdl3 sdl3-image raylib) \
    -lpthread -lm \
    -o myapp
