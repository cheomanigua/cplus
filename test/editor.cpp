#include "raylib.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

struct Block {
    Vector3 position;
    int typeID;
};

// Function to save the map to a plain text file
void SaveMap(const std::string& filename, const std::vector<Block>& blocks) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cout << "Error: Could not open file for saving: " << filename << "\n";
        return;
    }

    // Save total count first, then each block's data
    outFile << blocks.size() << "\n";
    for (const auto& block : blocks) {
        outFile << block.position.x << " " 
                << block.position.y << " " 
                << block.position.z << " " 
                << block.typeID << "\n";
    }
    outFile.close();
    std::cout << "Map successfully saved to " << filename << "\n";
}

// Function to load the map from a plain text file
void LoadMap(const std::string& filename, std::vector<Block>& blocks) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cout << "Error: Could not open file for loading: " << filename << "\n";
        return;
    }

    size_t count { 0 };
    inFile >> count;

    blocks.clear();
    for (size_t i { 0 }; i < count; ++i) {
        Block block {};
        inFile >> block.position.x >> block.position.y >> block.position.z >> block.typeID;
        blocks.push_back(block);
    }
    inFile.close();
    std::cout << "Map successfully loaded from " << filename << " (Total blocks: " << blocks.size() << ")\n";
}

int main() {
    const int screenWidth { 1280 };
    const int screenHeight { 720 };
    InitWindow(screenWidth, screenHeight, "Raylib Modular Map Editor - Save/Load");

    Camera3D camera {
        .position = Vector3{ 20.0f, 20.0f, 20.0f },
        .target = Vector3{ 0.0f, 0.0f, 0.0f },
        .up = Vector3{ 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    Model blockModels[3];
    blockModels[0] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    blockModels[1] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    blockModels[2] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    Image imgRed { GenImageColor(64, 64, RED) };
    Image imgGreen { GenImageColor(64, 64, GREEN) };
    Image imgBlue { GenImageColor(64, 64, BLUE) };

    Texture2D textures[3] = {
        LoadTextureFromImage(imgRed),
        LoadTextureFromImage(imgGreen),
        LoadTextureFromImage(imgBlue)
    };

    UnloadImage(imgRed);
    UnloadImage(imgGreen);
    UnloadImage(imgBlue);

    for (int i { 0 }; i < 3; ++i) {
        blockModels[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = textures[i];
    }

    const float gridSizeF { 1.0f };
    Vector3 cursorGridPos { 0.0f, 0.0f, 0.0f };
    std::vector<Block> placedBlocks {};
    int currentTypeID { 1 };
    const std::string saveFilename { "level_map.txt" };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // --- 1. UPDATE ---
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 mouseDelta { GetMouseDelta() };
            camera.position.x -= mouseDelta.x * 0.05f;
            camera.position.z -= mouseDelta.y * 0.05f;
        }

        if (IsKeyPressed(KEY_ONE)) currentTypeID = 1;
        if (IsKeyPressed(KEY_TWO)) currentTypeID = 2;
        if (IsKeyPressed(KEY_THREE)) currentTypeID = 3;

        // Save and Load Hotkeys
        if (IsKeyPressed(KEY_F5)) {
            SaveMap(saveFilename, placedBlocks);
        }
        if (IsKeyPressed(KEY_F6)) {
            LoadMap(saveFilename, placedBlocks);
        }

        if (IsKeyPressed(KEY_W)) cursorGridPos.z -= gridSizeF;
        if (IsKeyPressed(KEY_S)) cursorGridPos.z += gridSizeF;
        if (IsKeyPressed(KEY_A)) cursorGridPos.x -= gridSizeF;
        if (IsKeyPressed(KEY_D)) cursorGridPos.x += gridSizeF;
        if (IsKeyPressed(KEY_E)) cursorGridPos.y += gridSizeF;
        if (IsKeyPressed(KEY_Q) && cursorGridPos.y > 0) cursorGridPos.y -= gridSizeF;

        // Place Block
        if (IsKeyPressed(KEY_SPACE)) {
            bool occupied { false };
            for (const auto& b : placedBlocks) {
                if (b.position.x == cursorGridPos.x && 
                    b.position.y == cursorGridPos.y && 
                    b.position.z == cursorGridPos.z) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) {
                placedBlocks.push_back(Block{ cursorGridPos, currentTypeID });
            }
        }

        // Remove Block
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) {
            for (auto it { placedBlocks.begin() }; it != placedBlocks.end(); ++it) {
                if (it->position.x == cursorGridPos.x && 
                    it->position.y == cursorGridPos.y && 
                    it->position.z == cursorGridPos.z) {
                    placedBlocks.erase(it);
                    break;
                }
            }
        }

        // --- 2. DRAW ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

            DrawGrid(20, 1.0f);

            for (const auto& block : placedBlocks) {
                int modelIndex { (block.typeID >= 1 && block.typeID <= 3) ? block.typeID - 1 : 0 };
                DrawModel(blockModels[modelIndex], block.position, 1.0f, WHITE);
                DrawCubeWires(block.position, gridSizeF, gridSizeF, gridSizeF, BLACK);
            }

            DrawCubeWires(cursorGridPos, gridSizeF, gridSizeF, gridSizeF, DARKPURPLE);

        EndMode3D();

        // Draw UI Overlay
        DrawText("MODULAR MAP EDITOR (Save/Load)", 10, 10, 20, DARKGRAY);
        DrawText("Place: SPACE | Delete: BKSP | Save: F5 | Load: F6", 10, 40, 14, DARKGRAY);
        
        std::string typeText { "Current Type: " + std::to_string(currentTypeID) };
        DrawText(typeText.c_str(), 10, 70, 14, MAROON);
        
        std::string countText { "Total Blocks: " + std::to_string(placedBlocks.size()) };
        DrawText(countText.c_str(), 10, 95, 14, DARKGREEN);

        EndDrawing();
    }

    for (int i { 0 }; i < 3; ++i) {
        UnloadTexture(textures[i]);
        UnloadModel(blockModels[i]);
    }

    CloseWindow();
    return 0;
}
