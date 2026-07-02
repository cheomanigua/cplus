#pragma once
#include "SimMath.hpp"
#include <cstdint>

namespace SimEngine::Renderer {
    void Init(int width, int height, const char* title);
    bool ShouldClose();
    float GetFrameTime();
    void SetTargetFPS(int fps);
    void Begin();
    void End();
    void Close();
    void Clear(uint32_t color);
    void DrawCircle(Vec2 pos, float radius, uint32_t color);
    void DrawCircleLines(Vec2 pos, float radius, uint32_t color);
    void DrawText(const char* text, Vec2 pos, int fontSize, uint32_t color);
}
