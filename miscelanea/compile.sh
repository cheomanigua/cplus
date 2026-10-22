#!/bin/bash
set -e

g++ -std=c++23 \
    $(pkg-config --cflags sdl3 sdl3-image) \
    -c sdl3flecs.cpp -o main.o

g++ main.o flecs.o \
    $(pkg-config --libs sdl3 sdl3-image) \
    -lrt -lpthread -lm \
    -o myapp
