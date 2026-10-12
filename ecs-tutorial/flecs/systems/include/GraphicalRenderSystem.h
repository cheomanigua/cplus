#pragma once

#include <flecs.h>
#include "Position.h"

class GraphicalRenderSystem
{
public:
    explicit GraphicalRenderSystem(flecs::world& world);
    void update();

private:
    flecs::query<Position> positionQuery;
};
