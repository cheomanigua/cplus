g++ -std=c++23 -c prefab.cpp -o main.o
g++ main.o flecs.o $(pkg-config --libs raylib) -lrt -lpthread -lm -o myapp
