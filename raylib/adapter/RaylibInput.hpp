#pragma once
#include "raylib.h"
#include "SimInput.hpp"

namespace SimEngine::InputAdapter {
    inline InputIntent BackendPollInputs() noexcept {
        InputIntent intent;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            intent.moveRequested = true;
            Vector2 mouse = GetMousePosition();
            intent.destination = { mouse.x, mouse.y };
        }
        return intent;
    }
}
