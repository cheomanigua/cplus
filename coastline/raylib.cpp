
#include "raylib.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct Coastline {
    std::vector<Vector2> points;
};

struct Map {
    std::vector<Coastline> coastlines;
    Vector2 min{
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };
    Vector2 max{
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
    };

    [[nodiscard]] bool empty() const {
        return coastlines.empty();
    }
};

static bool ReadU32(std::istream& in, std::uint32_t& value)
{
    // The map format uses little-endian 32-bit integers.
    std::array<unsigned char, 4> bytes{};
    if (!in.read(reinterpret_cast<char*>(bytes.data()), bytes.size()))
        return false;

    value =
        static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8) |
        (static_cast<std::uint32_t>(bytes[2]) << 16) |
        (static_cast<std::uint32_t>(bytes[3]) << 24);

    return true;
}

static bool ReadFloat(std::istream& in, float& value)
{
    std::uint32_t bits{};
    if (!ReadU32(in, bits))
        return false;

    value = std::bit_cast<float>(bits);
    return std::isfinite(value);
}

static Map LoadMap(const std::string& filename)
{
    Map map;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "ERROR: Could not open " << filename << '\n';
        return map;
    }

    std::array<char, 4> magic{};
    std::uint32_t version{}, count{};

    if (!file.read(magic.data(), magic.size()) ||
        magic != std::array<char, 4>{'M', 'A', 'P', '2'} ||
        !ReadU32(file, version) || version != 1 ||
        !ReadU32(file, count)) {
        std::cerr << "ERROR: Invalid map header.\n";
        return map;
    }

    // Basic sanity check to avoid unreasonable allocations.
    constexpr std::uint32_t MaxCoastlines = 1'000'000;
    constexpr std::uint32_t MaxPointsPerCoastline = 10'000'000;

    if (count > MaxCoastlines) {
        std::cerr << "ERROR: Too many coastlines.\n";
        return map;
    }

    map.coastlines.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t n{};

        if (!ReadU32(file, n) || n > MaxPointsPerCoastline) {
            std::cerr << "ERROR: Invalid coastline point count.\n";
            return Map{};
        }

        Coastline line;
        line.points.reserve(n);

        for (std::uint32_t j = 0; j < n; ++j) {
            float x{}, y{};

            if (!ReadFloat(file, x) || !ReadFloat(file, y)) {
                std::cerr << "ERROR: Invalid or truncated map data.\n";
                return Map{};
            }

            line.points.push_back({x, y});

            map.min.x = std::min(map.min.x, x);
            map.min.y = std::min(map.min.y, y);
            map.max.x = std::max(map.max.x, x);
            map.max.y = std::max(map.max.y, y);
        }

        map.coastlines.push_back(std::move(line));
    }

    if (map.min.x == std::numeric_limits<float>::max()) {
        map.min = {0.0f, 0.0f};
        map.max = {0.0f, 0.0f};
    }

    std::cout << "Map loaded: " << map.coastlines.size()
              << " coastline segments\n";

    std::cout << "Size: "
              << (map.max.x - map.min.x) / 1000.0f
              << " x "
              << (map.max.y - map.min.y) / 1000.0f
              << " km\n";

    return map;
}

static Vector2 WorldToScreen(
    Vector2 world, Vector2 target, Vector2 offset, float zoom)
{
    return {
        offset.x + (world.x - target.x) * zoom,
        offset.y - (world.y - target.y) * zoom
    };
}

static Vector2 ScreenToWorld(
    Vector2 screen, Vector2 target, Vector2 offset, float zoom)
{
    return {
        target.x + (screen.x - offset.x) / zoom,
        target.y - (screen.y - offset.y) / zoom
    };
}

static void DrawMap(
    const Map& map, Vector2 target, Vector2 offset, float zoom)
{
    for (const Coastline& line : map.coastlines) {
        for (std::size_t j = 1; j < line.points.size(); ++j) {
            const Vector2 a = WorldToScreen(
                line.points[j - 1], target, offset, zoom);

            const Vector2 b = WorldToScreen(
                line.points[j], target, offset, zoom);

            DrawLineV(a, b, WHITE);
        }
    }
}

static void DrawWorldGrid(
    Vector2 target, Vector2 offset, float zoom,
    int width, int height)
{
    const float pxPerKm = zoom * 1000.0f;

    const float spacing =
        pxPerKm > 300 ? 100.0f :
        pxPerKm > 150 ? 250.0f :
        pxPerKm > 75  ? 500.0f :
        pxPerKm > 30  ? 1000.0f :
        pxPerKm > 10  ? 5000.0f :
        pxPerKm > 3   ? 10000.0f :
        pxPerKm > 1   ? 50000.0f : 100000.0f;

    const Vector2 tl = ScreenToWorld(
        {0, 0}, target, offset, zoom);

    const Vector2 br = ScreenToWorld(
        {static_cast<float>(width), static_cast<float>(height)},
        target, offset, zoom);

    const float minX = std::floor(tl.x / spacing) * spacing;
    const float maxX = std::ceil(br.x / spacing) * spacing;
    const float minY = std::floor(br.y / spacing) * spacing;
    const float maxY = std::ceil(tl.y / spacing) * spacing;

    const Color grid{60, 75, 90, 80};

    for (float x = minX; x <= maxX; x += spacing) {
        DrawLineV(
            WorldToScreen({x, minY}, target, offset, zoom),
            WorldToScreen({x, maxY}, target, offset, zoom),
            grid);
    }

    for (float y = minY; y <= maxY; y += spacing) {
        DrawLineV(
            WorldToScreen({minX, y}, target, offset, zoom),
            WorldToScreen({maxX, y}, target, offset, zoom),
            grid);
    }
}

static void DrawScaleBar(float zoom, int screenHeight)
{
    if (zoom <= 0.0f)
        return;

    constexpr float targetPixels = 150.0f;

    const float wanted = targetPixels / zoom;
    const float magnitude = std::pow(
        10.0f, std::floor(std::log10(wanted)));

    const float n = wanted / magnitude;

    float distance =
        n <= 1.0f ? magnitude :
        n <= 2.0f ? 2.0f * magnitude :
        n <= 5.0f ? 5.0f * magnitude :
                    10.0f * magnitude;

    float barPixels = distance * zoom;

    if (barPixels > 220.0f) {
        distance *= 0.5f;
        barPixels = distance * zoom;
    }

    if (barPixels < 60.0f) {
        distance *= 2.0f;
        barPixels = distance * zoom;
    }

    constexpr float x = 25.0f;
    const float y = static_cast<float>(screenHeight) - 45.0f;

    DrawRectangle(
        static_cast<int>(x - 10),
        static_cast<int>(y - 30),
        static_cast<int>(barPixels + 20),
        55,
        {0, 0, 0, 170});

    DrawLineEx({x, y}, {x + barPixels, y}, 4, WHITE);
    DrawLineEx({x, y - 7}, {x, y + 7}, 3, WHITE);
    DrawLineEx(
        {x + barPixels, y - 7},
        {x + barPixels, y + 7}, 3, WHITE);

    std::string label;

    if (distance < 1000.0f) {
        label = TextFormat("%.0f m", distance);
    } else {
        const float km = distance / 1000.0f;
        label = TextFormat(km >= 10.0f ? "%.0f km" : "%.1f km", km);
    }

    const int textWidth = MeasureText(label.c_str(), 18);

    DrawText(
        label.c_str(),
        static_cast<int>(x + barPixels * 0.5f - textWidth * 0.5f),
        static_cast<int>(y - 27),
        18,
        WHITE);
}

int main()
{
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Harpoon Map");
    SetTargetFPS(60);

    const Map map = LoadMap("east_asia.map");

    if (map.empty()) {
        CloseWindow();
        return 1;
    }

    const Vector2 center{
        (map.min.x + map.max.x) * 0.5f,
        (map.min.y + map.max.y) * 0.5f
    };

    const float mapWidth = map.max.x - map.min.x;
    const float mapHeight = map.max.y - map.min.y;

    Vector2 target = center;
    const Vector2 offset{
        screenWidth / 2.0f,
        screenHeight / 2.0f
    };

    const float zoomX =
        mapWidth > 0.0f ? screenWidth * 0.90f / mapWidth : 1.0f;

    const float zoomY =
        mapHeight > 0.0f ? screenHeight * 0.90f / mapHeight : 1.0f;

    float zoom = std::min(zoomX, zoomY);
    zoom = std::clamp(zoom, 0.000001f, 10.0f);

    const Vector2 shipPosition = center;
    constexpr float shipLength = 155.0f;
    constexpr float shipWidth = 20.0f;

    while (!WindowShouldClose()) {
        const float wheel = GetMouseWheelMove();

        if (wheel != 0.0f) {
            const Vector2 mouse = GetMousePosition();

            const Vector2 before =
                ScreenToWorld(mouse, target, offset, zoom);

            zoom *= wheel > 0.0f ? 1.15f : 1.0f / 1.15f;
            zoom = std::clamp(zoom, 0.000001f, 10.0f);

            const Vector2 after =
                ScreenToWorld(mouse, target, offset, zoom);

            target.x += before.x - after.x;
            target.y += before.y - after.y;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            const Vector2 delta = GetMouseDelta();

            target.x -= delta.x / zoom;
            target.y += delta.y / zoom;
        }

        BeginDrawing();
        ClearBackground({15, 35, 55, 255});

        DrawWorldGrid(
            target, offset, zoom, screenWidth, screenHeight);

        DrawMap(map, target, offset, zoom);

        const Vector2 ship = WorldToScreen(
            shipPosition, target, offset, zoom);

        const float sw = shipLength * zoom;
        const float sh = shipWidth * zoom;

        DrawRectangleRec(
            {ship.x - sw / 2.0f, ship.y - sh / 2.0f, sw, sh},
            RED);

        DrawRectangle(10, 10, 310, 180, {0, 0, 0, 160});
        DrawText("HARPOON MAP", 20, 20, 30, WHITE);
        DrawText("Middle mouse: pan", 20, 55, 20, WHITE);
        DrawText("Mouse wheel: zoom", 20, 80, 20, WHITE);
        DrawText(
            TextFormat("Zoom: %.6f px/m", zoom),
            20, 105, 20, WHITE);

        DrawText(
            TextFormat("Map: %.0f x %.0f km",
                mapWidth / 1000.0f, mapHeight / 1000.0f),
            20, 130, 20, WHITE);

        DrawText("Ship: 155m x 20m", 20, 155, 20, WHITE);

        DrawScaleBar(zoom, screenHeight);

        DrawLine(
            screenWidth - 60, 80,
            screenWidth - 60, 40, WHITE);

        DrawTriangle(
            {screenWidth - 60.0f, 30.0f},
            {screenWidth - 68.0f, 45.0f},
            {screenWidth - 52.0f, 45.0f},
            WHITE);

        DrawText("N", screenWidth - 67, 85, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

