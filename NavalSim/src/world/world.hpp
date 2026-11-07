#pragma once

#include <flecs.h>

class World
{
public:
    World() = default;
    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    flecs::world& ecs() noexcept;
    const flecs::world& ecs() const noexcept;

private:
    flecs::world world_;
};

