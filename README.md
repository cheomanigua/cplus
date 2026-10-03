# EnTT
Any project that requires `entt.hpp` must be downloaded manually into its proper directory because `entt.hpp` is ignored by `git`.

You can download `entt.hpp` from here:

[https://raw.githubusercontent.com/skypjack/entt/refs/heads/main/single_include/entt/entt.hpp](https://raw.githubusercontent.com/skypjack/entt/refs/heads/main/single_include/entt/entt.hpp)

You can also run the commands:

```
wget https://raw.githubusercontent.com/skypjack/entt/refs/heads/main/single_include/entt/entt.hpp
```

or

```
curl -L -o entt.hpp "https://raw.githubusercontent.com/skypjack/entt/refs/heads/main/single_include/entt/entt.hpp"
```

* * *

# Flecs

Any project that requires **Flecs** must follow these download and setup instructions:

## 1. Download

Download the following files into your project directory:

```
wget https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.h
wget https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.c

or

curl -L -o flecs.h "https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.h"
curl -L -o flecs.c "https://raw.githubusercontent.com/SanderMertens/flecs/master/distr/flecs.c"
```

## 2. Compile Flecs

Compile `flecs.c` into `flecs.o`. This only needs to be done once:

```
gcc -std=gnu99 -c flecs.c -o flecs.o
```

You can delete `flecs.c` after this if you don't need to rebuild Flecs. Keep `flecs.h` because your C++ source files need it when compiling.

## 3. Compile and build

Everytime you edit your source code, run these:

```
g++ -std=c++17 -c main.cpp -o main.o
g++ main.o flecs.o -lrt -lpthread -lm -o myapp
```

## 4. Run the executable

```
./myapp
```
