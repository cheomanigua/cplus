#pragma once
#include "entt.hpp"
#include "Position.h"
#include "Speed.h"
#include "Input.h"
#include "Direction.h"

class MovementSystem
{
public:
    explicit MovementSystem(entt::registry& registry);
    void update();

private:
    entt::registry& registry;
    entt::view<entt::get_t<Position, Speed, Input>> inputView;
    entt::view<entt::get_t<Position, Speed, Direction>, entt::exclude_t<Input>> autoView;
};
