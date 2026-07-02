#pragma once
#include "raylib.h"
#include "SimMath.hpp"

namespace SimEngine::Renderer {
    // Macro logic to decide if we define the function or just declare it
    #ifdef RAYLIB_RENDERER_IMPLEMENTATION
        #define RENDERER_FUNC
    #else
        #define RENDERER_FUNC inline
    #endif

    RENDERER_FUNC void Init(int w, int h, const char* t) { InitWindow(w, h, t); }
    RENDERER_FUNC bool ShouldClose() { return WindowShouldClose(); }
    RENDERER_FUNC float GetFrameTime() { return ::GetFrameTime(); }
    RENDERER_FUNC void SetTargetFPS(int fps) { ::SetTargetFPS(fps); }
    RENDERER_FUNC void Begin() { BeginDrawing(); }
    RENDERER_FUNC void End() { EndDrawing(); }
    RENDERER_FUNC void Close() { CloseWindow(); }
    RENDERER_FUNC void Clear(uint32_t color) { ClearBackground(GetColor(color)); }
    RENDERER_FUNC void DrawCircle(Vec2 pos, float radius, uint32_t color) { DrawCircleV(pos, radius, GetColor(color)); }
    RENDERER_FUNC void DrawCircleLines(Vec2 pos, float radius, uint32_t color) { DrawCircleLines((int)pos.x, (int)pos.y, radius, GetColor(color)); }
    RENDERER_FUNC void DrawText(const char* text, Vec2 pos, int fontSize, uint32_t color) {
        DrawText(text, (int)pos.x, (int)pos.y, fontSize, GetColor(color));
    }
}
