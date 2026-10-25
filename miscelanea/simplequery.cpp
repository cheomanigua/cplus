#include <cstddef>
#include <print>
#include "flecs.h"

struct Position {
    float x{}, y{};
};

void printPositions(flecs::query<Position>& query)
{
    query.each([](flecs::entity e, const Position& pos) {
        std::println("Entity {}: {}, {}", e.id(), pos.x, pos.y);
    });
}

int main()
{
    flecs::world world;

    for (std::size_t i = 0; i < 5; ++i) {
        world.entity().set<Position>({
            static_cast<float>(i),
            static_cast<float>(i)
        });
    }

    auto query = world.query_builder<Position>()
        .cached()
        .build();

    printPositions(query);
}
