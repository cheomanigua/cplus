#include "SimRenderer.hpp"

#if defined(BACKEND_RAYLIB)
    #define RAYLIB_RENDERER_IMPLEMENTATION
    #include "RaylibRenderer.hpp"
#elif defined(BACKEND_GODOT)
    #include "GodotRenderer.hpp"
#endif
