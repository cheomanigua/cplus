#include <cstddef>
#include <print>

#include "entt.hpp"

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

void movementSystem(entt::view<entt::get_t<Position, Velocity>>& v, float dt)
{
    for(auto [entity, pos, vel] : v.each()) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    }
}

void printPos(entt::view<entt::get_t<Position>>& v)
{
    for(auto [entity, pos] : v.each()) {
        std::println("Entity {}: {}, {}", entt::to_integral(entity), pos.x, pos.y);
    }
    std::println();
}

int main() {
    entt::registry reg{};
    constexpr float dt {1.0f / 60.0f};
    auto posV = reg.view<Position>();
    auto movV = reg.view<Position, Velocity>();

    for(std::size_t i = 0; i < 5; ++i) {
        auto entity = reg.create();
        float value = static_cast<float>(i);
        reg.emplace<Position>(entity, value, value);
        reg.emplace<Velocity>(entity, value, value);
    }
    printPos(posV);

    for(std::size_t frame = 0; frame < 60; ++frame) {
        movementSystem(movV, dt);
    }
    printPos(posV);

    reg.destroy(static_cast<entt::entity>(3));
    printPos(posV);

    auto entity = reg.create();
    reg.emplace<Position>(entity, 5.0f, 5.0f);
    printPos(posV);
}
