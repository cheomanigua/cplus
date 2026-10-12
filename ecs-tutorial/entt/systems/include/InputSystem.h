#pragma once

#include "entt.hpp"
#include "Position.h"
#include "SelectionBounds.h"
#include "Input.h"
#include "Tags.h"

class InputSystem
{
public:
    explicit InputSystem(entt::registry& registry);
    void update();

private:
    entt::registry& registry;

    // Entities that can be selected with the mouse
    entt::view<entt::get_t<Position, SelectionBounds>> selectionView;

    // Selected entities that can receive keyboard input
    entt::view<entt::get_t<Input, TagSelected>> inputView;
};
