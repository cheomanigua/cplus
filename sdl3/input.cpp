#include "input.h"

#include <SDL3/SDL.h>

Input GetInput()
{
    const bool* keyboard =
        SDL_GetKeyboardState(nullptr);

    return {
        keyboard[SDL_SCANCODE_LEFT],
        keyboard[SDL_SCANCODE_RIGHT],
        keyboard[SDL_SCANCODE_UP],
        keyboard[SDL_SCANCODE_DOWN]
    };
}
