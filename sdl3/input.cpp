#include "input.h"

#include <SDL3/SDL.h>

Input GetInput()
{
    const bool* keyboard =
        SDL_GetKeyboardState(nullptr);

    return {
        keyboard[SDL_SCANCODE_A],
        keyboard[SDL_SCANCODE_D],
        keyboard[SDL_SCANCODE_W],
        keyboard[SDL_SCANCODE_S]
    };
}
