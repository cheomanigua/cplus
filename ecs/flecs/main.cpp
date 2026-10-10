#include <print>
#include "flecs.h"

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

void movementSys(flecs::query<Position, Velocity>& q, float dt)
{
    q.each([dt](Position& pos, Velocity& vel) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    });
}

void printPos(flecs::query<const Position>& q)
{
    q.each([](flecs::entity e, const Position& pos) {
        std::println("Entity {}: {}, {}", e.id(), pos.x, pos.y);
    });
}

int main() {
    flecs::world world;
    constexpr float dt {1.0f / 60.0f};
    auto posQ = world.query<const Position>();
    auto movQ = world.query_builder<Position, Velocity>().cached().build();

    for(std::size_t i = 0; i < 5; ++i) {
        auto e = world.entity();
        float value = static_cast<float>(i);
        e.set<Position>({value, value});
        e.set<Velocity>({value, value});
    }
    printPos(posQ);

    for(std::size_t frame = 0; frame < 60; ++frame) {
        movementSys(movQ, dt);
    }
    printPos(posQ);
}
