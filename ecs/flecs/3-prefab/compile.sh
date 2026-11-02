g++ -std=c++20 -c main.cpp -o main.o
g++ main.o flecs.o -lpthread -lm -o myapp
