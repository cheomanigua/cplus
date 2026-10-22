#include <cstddef>
#include <print>

#include "flecs.h"

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

void printPos(flecs::query<const Position>& q)
{
    q.each([](flecs::entity e, const Position& pos) {
        std::println("Entity {}: {}, {}", e.id(), pos.x, pos.y);
    });
    std::println();
}

int main() {
    flecs::world world{};
	world.set_target_fps(60.0f);
    auto posQ = world.query<const Position>();

	// Create a system. Flecs manages its query and execution
	world.system<Position, const Velocity>("Movement")
        .each([](flecs::iter& it, std::size_t,
                 Position& pos, const Velocity& vel) {
            pos.x += vel.x * it.delta_time();
            pos.y += vel.y * it.delta_time();
        });

    for(std::size_t i = 0; i < 5; ++i) {
        auto e = world.entity();
        float value = static_cast<float>(i);
        e.set<Position>({value, value});
        e.set<Velocity>({value, value});
    }
    //printPos(posQ);

    while(true) {
        world.progress();
    }
    printPos(posQ);

    world.entity(523).destruct();
    printPos(posQ);

    auto e = world.entity();
    e.set<Position>({5.0f, 5.0f});
    printPos(posQ);
}
