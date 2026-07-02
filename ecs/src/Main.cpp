#include "Engine/EngineDriver.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/ConsoleGameView.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/PathResolver.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Tests/TestRunner.hpp"
#include "raylib.h"
#include <iostream>

int main() {

    TestRunner::RunAll();
    // 1. Initialize the DataLoader with the path to your data directory
    DataLoader loader;

    // 2. Load the manifest
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    std::cout << "Successfully loaded " << loader.GetItems().size() << " items." << std::endl;
    
    // 3. Initialize the FormulaProcessor 
    // This loads the math logic from formulas.json into memory
    FormulaProcessor::Initialize(Engine::GetDataPath("system/formulas.json"));

    // Add debug print here
    std::cout << "DEBUG: Checking Class lookup..." << std::endl;
    try {
        const auto& warrior = loader.GetClassData("Warrior");
        std::cout << "Found Warrior: Str=" << warrior.ClassStr << std::endl;
    } catch (...) {
        std::cout << "Warrior data missing from map!" << std::endl;
    }

    // 4. Setup View and Engine for initial processing
    ConsoleGameView consoleView; 
    
    // Inject the loader reference so the EngineDriver can perform 
    // blueprint lookups during command execution
    EngineDriver engine(&consoleView, loader, loader.GetItems(), "data");

    // 5. Retrieve NPCs from the loader and Queue UpdateStats commands
    const auto& npcs = loader.GetNPCs();
    std::cout << "Queueing UpdateStats for " << npcs.size() << " NPCs..." << std::endl;
    
    for (const auto& npc : npcs) {
        engine.AddCommand(GameCommand{ CommandType::UpdateStats, npc.EntityId });
    }

    // 6. Run the engine tick to process queued commands
    // This will trigger the FormulaProcessor logic for each queued entity
    engine.Tick(0.0f);

    // 7. Print results (Comparing Blueprint definition vs. Computed State)
    consoleView.RenderNPCStats(loader.GetNPCs(), engine);

    // 8. Initialize Raylib window for Graphical Mode
    InitWindow(800, 600, "Data Driven Engine");
    SetTargetFPS(60);

    // Use RaylibView for the main game loop
    RaylibGameView raylibView;
    // Note: Re-binding the engine to the new graphical view
    EngineDriver graphicsEngine(&raylibView, loader, loader.GetItems(), "data");

    // !!! CRITICAL: REGISTER YOUR NPCS SO THEY EXIST IN THE REGISTRY !!!
auto* registry = graphicsEngine.GetRegistry();
for (const auto& npc : loader.GetNPCs()) {
    // Manually register these so the drawing loop sees them
    registry->RegisterStats(npc.EntityId, { /* initial stats */ });
    
    // Set their starting position from your blueprint
    Vector2* pos = registry->GetPosition(npc.EntityId);
    if (pos) {
        *pos = npc.SpawnPosition;
    }
}

    // 9. Main Loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Engine Tick processes logic
        graphicsEngine.Tick(GetFrameTime());

        auto* registry = graphicsEngine.GetRegistry();
        int32_t count = registry->GetActiveCount();
        const auto& activeIds = registry->GetActiveEntities();
        
        // Draw loop
        for (int i = 0; i < count; i++) {
            int32_t id = activeIds[i];
            Vector2* pos = graphicsEngine.GetRegistry()->GetPosition(id);
            
            if (pos) {
                // This triggers your Raylib rendering logic using the updated Vector2 position
                raylibView.DrawMesh(id, *pos); 
            }
        }
        
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
