#gcc main.c -o map_viewer $(pkg-config --cflags --libs raylib) -lm

#g++ -std=c++20 -O2 raylib.cpp -o map_viewer \
#    $(pkg-config --cflags --libs raylib)

g++ -std=c++20 -O2 sdl3.cpp -o map_viewer \
    $(pkg-config --cflags --libs sdl3)
