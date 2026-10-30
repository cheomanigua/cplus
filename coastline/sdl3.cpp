#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

constexpr double TARGET_FRAME_TIME = 1.0 / 60.0;

// ------------------------------------------------------------
// Map data
// ------------------------------------------------------------

struct Coastline {
    std::vector<SDL_FPoint> points;
};

struct Map {
    std::vector<Coastline> coastlines;

    SDL_FPoint min{
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };

    SDL_FPoint max{
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
    };

    [[nodiscard]] bool empty() const {
        return coastlines.empty();
    }
};

static bool ReadU32(std::istream& in, std::uint32_t& value)
{
    std::array<unsigned char, 4> bytes{};

    if (!in.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size())))
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

    static_assert(sizeof(float) == sizeof(bits));

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

    if (!file.read(magic.data(),
                   static_cast<std::streamsize>(magic.size())) ||
        magic != std::array<char, 4>{'M', 'A', 'P', '2'} ||
        !ReadU32(file, version) || version != 1 ||
        !ReadU32(file, count)) {
        std::cerr << "ERROR: Invalid map header.\n";
        return map;
    }

    constexpr std::uint32_t MaxCoastlines = 1'000'000;
    constexpr std::uint32_t MaxPointsPerCoastline = 10'000'000;

    if (count > MaxCoastlines) {
        std::cerr << "ERROR: Too many coastlines.\n";
        return map;
    }

    map.coastlines.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t n{};

        if (!ReadU32(file, n) ||
            n > MaxPointsPerCoastline) {
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

// ------------------------------------------------------------
// World <-> screen coordinates
// World X increases east.
// World Y increases north.
// Screen Y increases downward.
// ------------------------------------------------------------

static SDL_FPoint WorldToScreen(
    SDL_FPoint world,
    SDL_FPoint target,
    SDL_FPoint offset,
    float zoom)
{
    return {
        offset.x + (world.x - target.x) * zoom,
        offset.y - (world.y - target.y) * zoom
    };
}

static SDL_FPoint ScreenToWorld(
    SDL_FPoint screen,
    SDL_FPoint target,
    SDL_FPoint offset,
    float zoom)
{
    return {
        target.x + (screen.x - offset.x) / zoom,
        target.y - (screen.y - offset.y) / zoom
    };
}

// ------------------------------------------------------------
// Drawing helpers
// ------------------------------------------------------------

static void SetColor(SDL_Renderer* renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(
        renderer, color.r, color.g, color.b, color.a);
}

static void DrawLine(
    SDL_Renderer* renderer,
    SDL_FPoint a,
    SDL_FPoint b)
{
    SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
}

static void DrawMap(
    SDL_Renderer* renderer,
    const Map& map,
    SDL_FPoint target,
    SDL_FPoint offset,
    float zoom)
{
    SetColor(renderer, {235, 235, 225, 255});

    for (const Coastline& line : map.coastlines) {
        for (std::size_t j = 1; j < line.points.size(); ++j) {
            const SDL_FPoint a = WorldToScreen(
                line.points[j - 1], target, offset, zoom);

            const SDL_FPoint b = WorldToScreen(
                line.points[j], target, offset, zoom);

            DrawLine(renderer, a, b);
        }
    }
}

static void DrawWorldGrid(
    SDL_Renderer* renderer,
    SDL_FPoint target,
    SDL_FPoint offset,
    float zoom,
    int width,
    int height)
{
    const float pxPerKm = zoom * 1000.0f;

    const float spacing =
        pxPerKm > 300 ? 100.0f :
        pxPerKm > 150 ? 250.0f :
        pxPerKm > 75  ? 500.0f :
        pxPerKm > 30  ? 1000.0f :
        pxPerKm > 10  ? 5000.0f :
        pxPerKm > 3   ? 10000.0f :
        pxPerKm > 1   ? 50000.0f :
                        100000.0f;

    const SDL_FPoint tl = ScreenToWorld(
        {0.0f, 0.0f}, target, offset, zoom);

    const SDL_FPoint br = ScreenToWorld(
        {static_cast<float>(width),
         static_cast<float>(height)},
        target, offset, zoom);

    const float minX = std::floor(tl.x / spacing) * spacing;
    const float maxX = std::ceil(br.x / spacing) * spacing;
    const float minY = std::floor(br.y / spacing) * spacing;
    const float maxY = std::ceil(tl.y / spacing) * spacing;

    SetColor(renderer, {60, 75, 90, 100});

    for (float x = minX; x <= maxX; x += spacing) {
        DrawLine(
            renderer,
            WorldToScreen({x, minY}, target, offset, zoom),
            WorldToScreen({x, maxY}, target, offset, zoom));
    }

    for (float y = minY; y <= maxY; y += spacing) {
        DrawLine(
            renderer,
            WorldToScreen({minX, y}, target, offset, zoom),
            WorldToScreen({maxX, y}, target, offset, zoom));
    }
}

static void DrawScaleBar(
    SDL_Renderer* renderer,
    float zoom,
    int screenHeight)
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

    SDL_FRect background{
        x - 10.0f, y - 30.0f,
        barPixels + 20.0f, 55.0f
    };

    SetColor(renderer, {0, 0, 0, 180});
    SDL_RenderFillRect(renderer, &background);

    SetColor(renderer, {255, 255, 255, 255});

    SDL_RenderLine(renderer, x, y, x + barPixels, y);
    SDL_RenderLine(renderer, x, y - 7, x, y + 7);
    SDL_RenderLine(
        renderer, x + barPixels, y - 7,
        x + barPixels, y + 7);

    char label[64];

    if (distance < 1000.0f) {
        std::snprintf(label, sizeof(label), "%.0f m", distance);
    } else {
        const float km = distance / 1000.0f;

        std::snprintf(
            label, sizeof(label),
            km >= 10.0f ? "%.0f km" : "%.1f km", km);
    }

    // SDL_RenderDebugText uses SDL's small built-in debug font.
    SDL_RenderDebugText(
        renderer, x, y - 25.0f, label);
}

static void DrawNorthIndicator(
    SDL_Renderer* renderer,
    int screenWidth)
{
    const float x = static_cast<float>(screenWidth) - 60.0f;

    SetColor(renderer, {255, 255, 255, 255});

    SDL_RenderLine(renderer, x, 80.0f, x, 43.0f);

    SDL_RenderLine(renderer, x, 30.0f, x - 8.0f, 45.0f);
    SDL_RenderLine(renderer, x - 8.0f, 45.0f, x + 8.0f, 45.0f);
    SDL_RenderLine(renderer, x + 8.0f, 45.0f, x, 30.0f);

    SDL_RenderDebugText(renderer, x - 4.0f, 85.0f, "N");
}

static void DrawInfoPanel(
    SDL_Renderer* renderer,
    float zoom,
    float mapWidth,
    float mapHeight)
{
    SDL_FRect panel{10.0f, 10.0f, 310.0f, 180.0f};

    SetColor(renderer, {0, 0, 0, 180});
    SDL_RenderFillRect(renderer, &panel);

    SetColor(renderer, {255, 255, 255, 255});

    char text[128];

    SDL_RenderDebugText(renderer, 20.0f, 20.0f, "HARPOON MAP");
    SDL_RenderDebugText(renderer, 20.0f, 40.0f, "Middle mouse: pan");
    SDL_RenderDebugText(renderer, 20.0f, 55.0f, "Mouse wheel: zoom");

    std::snprintf(text, sizeof(text), "Zoom: %.6f px/m", zoom);
    SDL_RenderDebugText(renderer, 20.0f, 75.0f, text);

    std::snprintf(
        text, sizeof(text), "Map: %.0f x %.0f km",
        mapWidth / 1000.0f, mapHeight / 1000.0f);

    SDL_RenderDebugText(renderer, 20.0f, 95.0f, text);
    SDL_RenderDebugText(renderer, 20.0f, 115.0f, "Ship: 155m x 20m");
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main()
{
    constexpr int initialWidth = 1280;
    constexpr int initialHeight = 720;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Harpoon Map",
        initialWidth,
        initialHeight,
        SDL_WINDOW_RESIZABLE);

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: "
                  << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: "
                  << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const Map map = LoadMap("east_asia.map");

    if (map.empty()) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const SDL_FPoint center{
        (map.min.x + map.max.x) * 0.5f,
        (map.min.y + map.max.y) * 0.5f
    };

    const float mapWidth = map.max.x - map.min.x;
    const float mapHeight = map.max.y - map.min.y;

    SDL_FPoint target = center;

    int screenWidth = initialWidth;
    int screenHeight = initialHeight;

    SDL_FPoint offset{
        screenWidth / 2.0f,
        screenHeight / 2.0f
    };

    const float zoomX =
        mapWidth > 0.0f ? screenWidth * 0.90f / mapWidth : 1.0f;

    const float zoomY =
        mapHeight > 0.0f ? screenHeight * 0.90f / mapHeight : 1.0f;

    float zoom = std::clamp(
        std::min(zoomX, zoomY), 0.000001f, 10.0f);

    const SDL_FPoint shipPosition = center;
    constexpr float shipLength = 155.0f;
    constexpr float shipWidth = 20.0f;

	const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();

    bool running = true;
    SDL_Event event{};

    while (running) {
		const Uint64 frameStart = SDL_GetPerformanceCounter();
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    running = false;
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                float mx{}, my{};
                SDL_GetMouseState(&mx, &my);

                const SDL_FPoint mouse{mx, my};

                const SDL_FPoint before =
                    ScreenToWorld(mouse, target, offset, zoom);

                // Smooth exponential zoom.
                zoom *= std::exp(
                    static_cast<float>(event.wheel.y) * 0.14f);

                zoom = std::clamp(zoom, 0.000001f, 10.0f);

                const SDL_FPoint after =
                    ScreenToWorld(mouse, target, offset, zoom);

                target.x += before.x - after.x;
                target.y += before.y - after.y;
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                const SDL_MouseButtonFlags buttons =
                    SDL_GetMouseState(nullptr, nullptr);

                if (buttons & SDL_BUTTON_MMASK) {
                    target.x -= event.motion.xrel / zoom;
                    target.y += event.motion.yrel / zoom;
                }
                break;
            }

            default:
                break;
            }
        }

        SDL_GetRenderOutputSize(
            renderer, &screenWidth, &screenHeight);

        offset = {
            screenWidth / 2.0f,
            screenHeight / 2.0f
        };

        SetColor(renderer, {15, 35, 55, 255});
        SDL_RenderClear(renderer);

        DrawWorldGrid(
            renderer, target, offset, zoom,
            screenWidth, screenHeight);

        DrawMap(renderer, map, target, offset, zoom);

        // Ship: rectangular placeholder, centered on the world position.
        const SDL_FPoint ship = WorldToScreen(
            shipPosition, target, offset, zoom);

        const float sw = shipLength * zoom;
        const float sh = shipWidth * zoom;

        SDL_FRect shipRect{
            ship.x - sw * 0.5f,
            ship.y - sh * 0.5f,
            sw,
            sh
        };

        SetColor(renderer, {230, 50, 50, 255});
        SDL_RenderFillRect(renderer, &shipRect);

        DrawInfoPanel(renderer, zoom, mapWidth, mapHeight);
        DrawScaleBar(renderer, zoom, screenHeight);
        DrawNorthIndicator(renderer, screenWidth);

        SDL_RenderPresent(renderer);

		const Uint64 frameEnd = SDL_GetPerformanceCounter();

		const double frameTime =
			static_cast<double>(frameEnd - frameStart) /
			static_cast<double>(performanceFrequency);

		const double remaining = TARGET_FRAME_TIME - frameTime;

		if (remaining > 0.0)
		{
			const Uint32 delayMilliseconds =
				static_cast<Uint32>(remaining * 1000.0);

			if (delayMilliseconds > 0)
			{
				SDL_Delay(delayMilliseconds);
			}
		}
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
