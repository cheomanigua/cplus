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
    constexpr float deltaTime {1.0f / 60.0f};
    auto posQ = world.query<const Position>();

	// Create 5 entities
    for(std::size_t i = 0; i < 5; ++i) {
        auto e = world.entity();
        float value = static_cast<float>(i);
        e.set<Position>({value, value});
        e.set<Velocity>({value, value});
    }
    printPos(posQ);

	// Movement system
	world.system<Position, const Velocity>("Movement")
		.each([](Position& p, const Velocity& v) {
				p.x += v.x * deltaTime;
				p.y += v.y * deltaTime;
				});

	// Simulate loop
    for(std::size_t frame = 0; frame < 60; ++frame) {
        world.progress(deltaTime);
    }
    printPos(posQ);

    world.entity(521).destruct();
    printPos(posQ);

    auto e = world.entity();
    e.set<Position>({5.0f, 5.0f});
    printPos(posQ);
}
