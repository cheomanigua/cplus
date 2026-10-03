## 1. Download Flecs distribution files

```bash
mkdir myproject
cd myproject

wget https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.h
wget https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.c

or

curl -L -o flecs.h "https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.h"
curl -L -o flecs.c "https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.c"
```

> For a reproducible project, replace `master` with a specific Flecs release/tag.

## 2. Compile Flecs

Compile `flecs.c` into `flecs.o`. This only needs to be done once:

```bash
gcc -std=gnu99 -c flecs.c -o flecs.o
```

You can delete `flecs.c` after this if you don't need to rebuild Flecs. Keep `flecs.h` because your C++ source files need it when compiling.

## 3. Compile and build

Run these commands every time you edit `main.cpp`:

```bash
g++ -std=c++17 -c main.cpp -o main.o
g++ main.o flecs.o -lrt -lpthread -lm -o myapp
```

## 4. Run the executable

```bash
./myapp
```

The **`entities[3].destruct()` change is the most important one**. Your `520` happened to correspond to one of the entities in your particular run, but you shouldn't rely on that ID.

