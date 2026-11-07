#include "world/world.hpp"

flecs::world& World::ecs() noexcept
{
    return world_;
}

const flecs::world& World::ecs() const noexcept
{
    return world_;
}

