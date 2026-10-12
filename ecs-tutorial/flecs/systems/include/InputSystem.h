#pragma once

#include <flecs.h>

#include "Input.h"
#include "Position.h"
#include "SelectionBounds.h"
#include "Tags.h"

class InputSystem
{
public:
    explicit InputSystem(flecs::world& world);
    void update();

private:
    flecs::world& world;
    flecs::query<Position, SelectionBounds> selectionQuery;
    flecs::query<Input, TagSelected> inputQuery;
};
