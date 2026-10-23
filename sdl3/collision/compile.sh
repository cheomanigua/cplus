#!/bin/bash
set -e

g++ -std=c++23 \
    $(pkg-config --cflags sdl3 sdl3-ttf) \
    *.cpp \
    $(pkg-config --libs sdl3 sdl3-ttf) \
    -o myapp
