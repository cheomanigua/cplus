#pragma once

// Tell raymath NOT to redefine types that raylib already provides
#define RAYMATH_IMPLEMENTATION
#define RAYMATH_STANDALONE
#include "raylib.h" 
#include "raymath.h"

namespace MathBackend {
    using Vec2 = Vector2;
    inline Vec2 Add(Vec2 a, Vec2 b) { return Vector2Add(a, b); }
    inline Vec2 Subtract(Vec2 a, Vec2 b) { return Vector2Subtract(a, b); }
    inline Vec2 MoveTowards(Vec2 current, Vec2 target, float maxDist) { return Vector2MoveTowards(current, target, maxDist); }
    inline bool Equals(Vec2 a, Vec2 b) { return Vector2Equals(a, b); }
}
