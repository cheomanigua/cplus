g++ -std=c++23 *.cpp flecs.o \
    $(pkg-config --cflags --libs sdl3 sdl3-image) \
    -lpthread -lm \
    -o game
