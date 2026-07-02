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

    // 1. Initialize the DataLoader
    DataLoader loader;
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    std::cout << "Successfully loaded " << loader.GetItems().size() << " items." << std::endl;
    
    // 2. Initialize FormulaProcessor
    FormulaProcessor::Initialize(Engine::GetDataPath("system/formulas.json"));

    // 3. Create the SHARED Registry (Passing items to constructor)
    EntityRegistry sharedRegistry(loader.GetItems());

    // 4. Setup Views
    ConsoleGameView consoleView; 
    
    // 5. Initialize both Engines with the SHARED Registry
    EngineDriver consoleEngine(&consoleView, loader, loader.GetItems(), "data", &sharedRegistry);
    
    // 6. Initialize Raylib and the Graphics Engine
    InitWindow(800, 600, "Data Driven Engine");
    SetTargetFPS(60);
    RaylibGameView raylibView;
    EngineDriver graphicsEngine(&raylibView, loader, loader.GetItems(), "data", &sharedRegistry);

    // 7. Retrieve NPCs and Queue Commands
    const auto& npcs = loader.GetNPCs();
    std::cout << "Queueing UpdateStats for " << npcs.size() << " NPCs..." << std::endl;

    consoleEngine.InitializeEntities(loader);
    
    for (const auto& npc : npcs) {
        // Prepare stats for registration
        EntityStats stats; 
        stats.IsDirty = true;
        
        // Register entities into the shared registry
        sharedRegistry.RegisterStats(npc.EntityId, stats);
        
        // Command the console engine to perform the heavy math logic
        consoleEngine.AddCommand(GameCommand{ CommandType::UpdateStats, npc.EntityId });
    }

    // 8. Run the engine tick on the console engine to process calculations
    consoleEngine.Tick(0.0f);

    // 9. Print results via console
    consoleView.RenderNPCStats(loader.GetNPCs(), consoleEngine);

    // 10. Main Loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Engine Tick processes logic (uses shared registry internally)
        graphicsEngine.Tick(GetFrameTime());

        // Draw loop using the shared registry via the graphicsEngine
        auto* registry = graphicsEngine.GetRegistry();
        int32_t count = registry->GetActiveCount();
        const auto& activeIds = registry->GetActiveEntities();
        
        for (int i = 0; i < count; i++) {
            int32_t id = activeIds[i];
            Vector2* pos = registry->GetPosition(id);
            
            if (pos) {
                // Now that registry is shared, graphicsEngine finds the positions
                // calculated by the consoleEngine's tick
                raylibView.DrawMesh(id, *pos); 
            }
        }
        
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
