#pragma once
#include "entt.hpp"
#include "Position.h"

class GraphicalRenderSystem
{
public:
    explicit GraphicalRenderSystem(entt::registry& registry);
    void update();

private:
    entt::view<entt::get_t<Position>> positionView;
};
