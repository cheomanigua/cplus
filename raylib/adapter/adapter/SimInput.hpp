#pragma once
#include "SimMath.hpp"

namespace SimEngine {
    struct InputIntent {
        bool moveRequested = false;
        Vec2 destination = {0, 0};
        bool radarToggleRequested = false;
    };

    namespace InputAdapter {
        // Just the declaration. No function body here!
        InputIntent PollInputs() noexcept;
    }
}
