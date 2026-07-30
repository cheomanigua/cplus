#pragma once

// The simulation uses these defines to swap backends at compile-time
#if defined(BACKEND_RAYLIB)
    #include "RaylibMath.hpp"
#elif defined(BACKEND_GODOT)
    #include "GodotMath.hpp"
#endif

namespace SimEngine {
    // We use the alias provided by the specific backend header
    using Vec2 = MathBackend::Vec2;

    namespace Math {
        inline Vec2 Add(const Vec2& a, const Vec2& b) noexcept { return MathBackend::Add(a, b); }
        inline Vec2 Subtract(const Vec2& a, const Vec2& b) noexcept { return MathBackend::Subtract(a, b); }
        inline Vec2 MoveTowards(const Vec2& current, const Vec2& target, float maxDist) noexcept {
            return MathBackend::MoveTowards(current, target, maxDist);
        }
        inline bool Equals(const Vec2& a, const Vec2& b) noexcept {
            return MathBackend::Equals(a, b);
        }
    }
}
