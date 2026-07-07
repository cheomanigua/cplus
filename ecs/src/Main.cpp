#include "Engine/EngineDriver.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/ConsoleGameView.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/PathResolver.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Systems/InputSystem.hpp"
#include "Tests/TestRunner.hpp"
#include "raylib.h"
#include <iostream>

constexpr int32_t PLAYER_ID = 1;

int main() {
    TestRunner::RunAll();

    // 1. Initialize the DataLoader
    DataLoader loader;
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    
    // 2. Initialize FormulaProcessor
    FormulaProcessor::Initialize(Engine::GetDataPath("system/formulas.json"));

    // 3. Create the SHARED Registry
    EntityRegistry sharedRegistry(loader.GetItems());

    // 4. Setup Views
    ConsoleGameView consoleView; 
    
    // 5. Initialize Engines
    EngineDriver consoleEngine(&consoleView, loader, loader.GetItems(), "/data", &sharedRegistry);
    
    InitWindow(800, 600, "Data Driven Engine");
    SetTargetFPS(60);
    RaylibGameView raylibView;
    raylibView.SetRegistry(&sharedRegistry);
    EngineFacade::Implementation = &raylibView;
    EngineDriver graphicsEngine(&raylibView, loader, loader.GetItems(), "/data", &sharedRegistry);

    // 6. SPAWN NPCs (Factory Pattern)
    // We no longer use loader.GetNPCs() to get IDs; we use templates to spawn entities.
    const auto& npcTemplates = loader.GetNPCTemplates(); 
    std::cout << "Spawning " << npcTemplates.size() << " NPCs via factory..." << std::endl;
    
    for (const auto& templateData : npcTemplates) {
        // The Registry handles _nextId++ and returns the unique ID for us
        int32_t newId = sharedRegistry.SpawnNPC(templateData);
        
        // Use the returned ID to command the engine logic
        consoleEngine.AddCommand(GameCommand{ CommandType::UpdateStats, newId });
    }

    // 7. Run engine tick
    consoleEngine.Tick(0.0f);

    consoleView.DisplayFullCharacterSheet(sharedRegistry, loader);

    // 8. Main Loop
    while (!WindowShouldClose()) {
        InputSystem::PollInput(graphicsEngine.GetCommandQueue(), PLAYER_ID, sharedRegistry);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        graphicsEngine.Tick(GetFrameTime());

        // Draw loop using the shared registry
        auto* registry = graphicsEngine.GetRegistry();
        const auto& activeIds = registry->GetActiveEntities();
        
        for (int i = 0; i < registry->GetActiveCount(); i++) {
            int32_t id = activeIds[i];
            Vector2* pos = registry->GetPosition(id);
            
            if (pos) {
                raylibView.DrawMesh(id, *pos); 
            }
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
