#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <array>

// ********************************************************************************** //
// Carefully analyze your png file so you can enter the correct constant values below //
// ********************************************************************************** //
constexpr int MAP_WIDTH        {64};       // Map size in tiles
constexpr int MAP_HEIGHT       {64};       // Map size in tiles
constexpr int SRC_TILE_SIZE    {32};       // Size drawn on the map in pixels
constexpr int SRC_TILE_SPACING {4};        // Spacing between tiles in tileset file
constexpr int ATLAS_COLS       {14};       // columns tileset file
constexpr int ATLAS_ROWS       {12};       // rows tileset file

int main()
{
    InitWindow(1400, 900, "Raylib Tilemap Editor - Modern C++ Advanced Optimization");
    SetTargetFPS(60);

    Texture2D tileset = LoadTexture("colored_tilemap.png");

    if (tileset.id == 0)
    {
        TraceLog(LOG_ERROR, "Could not load colored_tilemap.png");
        CloseWindow();
        return 1;
    }

    std::cout << "Tileset: " << tileset.width << " x " << tileset.height << "\n";

    // Two map layers: Layer 0 (Background), Layer 1 (Foreground)
    std::array<std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT>, 2> map;
    for (auto& layer : map)
    {
        for (auto& row : layer)
        {
            row.fill(-1);
        }
    }

    int currentLayer{0}; // 0 = Bottom layer, 1 = Top layer

    Camera2D camera{{0, 0}, {0, 0}, 0.0f, 1.0f};

    int selectedTile{0};

    // Cache the palette UI to a RenderTexture to avoid redrawing every icon from scratch each frame[cite: 2]
    RenderTexture2D paletteTarget = LoadRenderTexture(ATLAS_COLS * 15, ATLAS_ROWS * 15);
    BeginTextureMode(paletteTarget);
    ClearBackground(GRAY);
    for (int i{0}; i < ATLAS_COLS * ATLAS_ROWS; ++i)
    {
        int x{i % ATLAS_COLS};
        int y{i / ATLAS_COLS};

        Rectangle dst{
            static_cast<float>(x * 15),
            static_cast<float>(y * 15),
            14.0f,
            14.0f
        };

        Rectangle src{
            static_cast<float>(x * (SRC_TILE_SIZE + SRC_TILE_SPACING)),
            static_cast<float>(y * (SRC_TILE_SIZE + SRC_TILE_SPACING)),
            static_cast<float>(SRC_TILE_SIZE),
            static_cast<float>(SRC_TILE_SIZE)
        };

        DrawTexturePro(tileset, src, dst, {0, 0}, 0.0f, WHITE);
    }
    EndTextureMode();

    // RenderTexture for baking the map to eliminate thousands of draw calls per frame
    RenderTexture2D mapCanvas = LoadRenderTexture(MAP_WIDTH * SRC_TILE_SIZE, MAP_HEIGHT * SRC_TILE_SIZE);
    bool mapDirty{true}; // Flag to rebuild canvas only when changes occur

    while (!WindowShouldClose())
    {
        //--------------------------------------------------
        // Camera
        //--------------------------------------------------

        camera.zoom += GetMouseWheelMove() * 0.1f;

        if (camera.zoom < 0.25f)
            camera.zoom = 0.25f;

        if (camera.zoom > 8.0f)
            camera.zoom = 8.0f;

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 delta{GetMouseDelta()};
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        // Switch layers with Tab key
        if (IsKeyPressed(KEY_TAB))
        {
            currentLayer = 1 - currentLayer;
        }

        //--------------------------------------------------
        // Mouse position
        //--------------------------------------------------

        Vector2 mousePos{GetMousePosition()};
        Vector2 world{GetScreenToWorld2D(mousePos, camera)};

        int tx{static_cast<int>(world.x / SRC_TILE_SIZE)};
        int ty{static_cast<int>(world.y / SRC_TILE_SIZE)};

        bool mouseOverPalette{mousePos.x >= 1080.0f};

        //--------------------------------------------------
        // Palette selection
        //--------------------------------------------------

        if (mouseOverPalette && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            int px{(static_cast<int>(mousePos.x) - 1090) / 15};
            int py{(static_cast<int>(mousePos.y) - 40) / 15};

            if (px >= 0 && px < ATLAS_COLS &&
                py >= 0 && py < ATLAS_ROWS)
            {
                selectedTile = py * ATLAS_COLS + px;
            }
        }

        //--------------------------------------------------
        // Paint (on currentLayer)
        //--------------------------------------------------

        if (!mouseOverPalette &&
            tx >= 0 && tx < MAP_WIDTH &&
            ty >= 0 && ty < MAP_HEIGHT)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                if (map[currentLayer][ty][tx] != selectedTile)
                {
                    map[currentLayer][ty][tx] = selectedTile;
                    mapDirty = true; // Mark map canvas for update
                }
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                if (map[currentLayer][ty][tx] != -1)
                {
                    map[currentLayer][ty][tx] = -1;
                    mapDirty = true; // Mark map canvas for update
                }
            }
        }

        //--------------------------------------------------
        // Save (Using Modern C++ streams)
        //--------------------------------------------------

        if (IsKeyPressed(KEY_S))
        {
            std::ofstream outFile{"map.txt"};
            if (outFile.is_open())
            {
                for (int l{0}; l < 2; ++l)
                {
                    for (int y{0}; y < MAP_HEIGHT; ++y)
                    {
                        for (int x{0}; x < MAP_WIDTH; ++x)
                            outFile << map[l][y][x] << " ";

                        outFile << "\n";
                    }
                }
                std::cout << "Saved map.txt (2 layers)\n";
            }
        }

        //--------------------------------------------------
        // Load (Using Modern C++ streams)
        //--------------------------------------------------

        if (IsKeyPressed(KEY_L))
        {
            std::ifstream inFile{"map.txt"};
            if (inFile.is_open())
            {
                for (int l{0}; l < 2; ++l)
                    for (int y{0}; y < MAP_HEIGHT; ++y)
                        for (int x{0}; x < MAP_WIDTH; ++x)
                            inFile >> map[l][y][x];

                mapDirty = true; // Refresh canvas with loaded map data
                std::cout << "Loaded map.txt (2 layers)\n";
            }
        }

        //--------------------------------------------------
        // Re-bake Map Canvas if Modified (Dirty Flag Optimization)
        //--------------------------------------------------

        if (mapDirty)
        {
            BeginTextureMode(mapCanvas);
            ClearBackground(BLANK);

            for (int y{0}; y < MAP_HEIGHT; ++y)
            {
                for (int x{0}; x < MAP_WIDTH; ++x)
                {
                    bool hasTile{(map[0][y][x] >= 0) || (map[1][y][x] >= 0)};
                    if (!hasTile) continue;

                    Rectangle dst{
                        static_cast<float>(x * SRC_TILE_SIZE),
                        static_cast<float>(y * SRC_TILE_SIZE),
                        static_cast<float>(SRC_TILE_SIZE),
                        static_cast<float>(SRC_TILE_SIZE)
                    };

                    for (int l{0}; l < 2; ++l)
                    {
                        int tile{map[l][y][x]};

                        if (tile >= 0)
                        {
                            int col{tile % ATLAS_COLS};
                            int row{tile / ATLAS_COLS};
                            
                            Rectangle src{
                                static_cast<float>(col * (SRC_TILE_SIZE + SRC_TILE_SPACING)),
                                static_cast<float>(row * (SRC_TILE_SIZE + SRC_TILE_SPACING)),
                                static_cast<float>(SRC_TILE_SIZE),
                                static_cast<float>(SRC_TILE_SIZE)
                            };

                            DrawTexturePro(tileset, src, dst, {0, 0}, 0.0f, WHITE);
                        }
                    }
                }
            }
            EndTextureMode();
            mapDirty = false;
        }

        //--------------------------------------------------
        // Draw
        //--------------------------------------------------

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        // 1. Draw baked map canvas texture in a single draw call instead of thousands
        DrawTextureRec(
            mapCanvas.texture,
            Rectangle{0.0f, 0.0f, static_cast<float>(mapCanvas.texture.width), static_cast<float>(-mapCanvas.texture.height)},
            Vector2{0.0f, 0.0f},
            WHITE
        );

        // 2. View-Frustum Culling & Conditional Grid Rendering
        float screenWidth{1080.0f}; // Width allocated to the map view
        float screenHeight{900.0f};

        Vector2 minWorld{GetScreenToWorld2D({0.0f, 0.0f}, camera)};
        Vector2 maxWorld{GetScreenToWorld2D({screenWidth, screenHeight}, camera)};

        int startX{static_cast<int>(minWorld.x / SRC_TILE_SIZE) - 1};
        int endX{static_cast<int>(maxWorld.x / SRC_TILE_SIZE) + 1};
        int startY{static_cast<int>(minWorld.y / SRC_TILE_SIZE) - 1};
        int endY{static_cast<int>(maxWorld.y / SRC_TILE_SIZE) + 1};

        if (startX < 0) startX = 0;
        if (endX > MAP_WIDTH) endX = MAP_WIDTH;
        if (startY < 0) startY = 0;
        if (endY > MAP_HEIGHT) endY = MAP_HEIGHT;

        // Only draw grid lines if zoomed in close enough to distinguish individual cells
        if (camera.zoom > 0.5f)
        {
            for (int y{startY}; y < endY; ++y)
            {
                for (int x{startX}; x < endX; ++x)
                {
                    Rectangle dst{
                        static_cast<float>(x * SRC_TILE_SIZE),
                        static_cast<float>(y * SRC_TILE_SIZE),
                        static_cast<float>(SRC_TILE_SIZE),
                        static_cast<float>(SRC_TILE_SIZE)
                    };

                    DrawRectangleLinesEx(dst, 1, Fade(LIGHTGRAY, 0.15f));
                }
            }
        }

        EndMode2D();

        //--------------------------------------------------
        // Palette & UI info panel
        //--------------------------------------------------

        DrawRectangle(1080, 0, 320, 900, GRAY);
        DrawText("Palette", 1100, 10, 20, WHITE);

        // Draw cached palette texture[cite: 2]
        DrawTextureRec(
            paletteTarget.texture, 
            Rectangle{0.0f, 0.0f, static_cast<float>(paletteTarget.texture.width), static_cast<float>(-paletteTarget.texture.height)}, 
            Vector2{1090.0f, 40.0f}, 
            WHITE
        );

        // Highlight selected tile in the palette
        int selX{selectedTile % ATLAS_COLS};
        int selY{selectedTile / ATLAS_COLS};
        Rectangle selRect{
            static_cast<float>(1090 + selX * 15),
            static_cast<float>(40 + selY * 15),
            14.0f,
            14.0f
        };
        DrawRectangleLinesEx(selRect, 2, YELLOW);

        // Display current active layer info
        std::string layerText{"Layer: " + std::to_string(currentLayer) + " " + (currentLayer == 0 ? "(Bottom)" : "(Top)")};
        DrawText(layerText.c_str(), 1100, 440, 20, currentLayer == 0 ? WHITE : YELLOW);

        DrawText("TAB: Switch Layer", 1100, 485, 20, WHITE);
        DrawText("LMB: Paint",        1100, 510, 20, WHITE);
        DrawText("RMB: Erase",        1100, 535, 20, WHITE);
        DrawText("MMB: Pan",          1100, 560, 20, WHITE);
        DrawText("Wheel: Zoom",       1100, 585, 20, WHITE);
        DrawText("S: Save",           1100, 610, 20, WHITE);
        DrawText("L: Load",           1100, 635, 20, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(mapCanvas);
    UnloadRenderTexture(paletteTarget);
    UnloadTexture(tileset);
    CloseWindow();

    return 0;
}
