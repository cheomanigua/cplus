#pragma once
#include <flecs.h>
#include "Position.h"
#include "Speed.h"
#include "Input.h"
#include "Direction.h"

class MovementSystem
{
public:
    explicit MovementSystem(flecs::world& world);
    void update();

private:
    flecs::query<Position, Speed, Input> inputQuery;
    flecs::query<Position, Speed, Direction> autoQuery;
};
