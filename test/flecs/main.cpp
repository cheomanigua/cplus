#include <iostream>
#include "flecs.h"

struct Position {
    float x{}, y{};
};

void printPos(flecs::world& world)
{
    auto query = world.query<Position>();
    query.each([](flecs::entity entity, Position& pos) {
        std::cout << "Entity "
                  << entity.id() << ": "
                  << pos.x << ", "
                  << pos.y << "\n";
    });
    std::cout << "\n";
}

int main() {
    flecs::world world;

    for (std::size_t i = 0; i < 5; ++i) {
        auto entity = world.entity();

        entity.set<Position>({
            static_cast<float>(i),
            static_cast<float>(i)
        });
    }
    printPos(world);

    world.entity(520).destruct();
    printPos(world);

    auto entity = world.entity();
    entity.set<Position>({5.0f, 5.0f});
    printPos(world);
}
