#include "Engine/EngineDriver.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/ConsoleGameView.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/PathResolver.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Core/Commands/GameCommand.hpp"
#include "Systems/InputSystem.hpp"
#include "Tests/TestRunner.hpp"
#include <iostream>

#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"

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
    
    // Change the loop to use structured bindings for key-value pairs
    for (const auto& [name, templateData] : npcTemplates) {
        // 'name' is the key (e.g., "Thrall"), 'templateData' is the NPCBlueprint struct
        
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
        // 1. Maintain Spatial State (Required for InputSystem to find clicks)
        sharedRegistry.ClearGrid(); 
        for(int32_t id : sharedRegistry.GetActiveEntities()) {
            Vector2* pos = sharedRegistry.GetPosition(id);
            if(pos) sharedRegistry.UpdateEntityCell(id, *pos);
        }
    
        // 2. InputSystem handles Selection AND Movement Commands
        // No longer calling raylibView.GetNextCommand() here!
        InputSystem::PollInput(graphicsEngine.GetCommandQueue(), sharedRegistry);
        
        // 3. Engine updates logic
        graphicsEngine.Tick(GetFrameTime());
        
        // 4. Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Access movement component from the engine to pass to DrawMesh
        const auto& moveComp = graphicsEngine.GetMovementComponent();
        
        for (int i = 0; i < sharedRegistry.GetActiveCount(); i++) {
            int32_t id = sharedRegistry.GetActiveEntities()[i];
            Vector2* pos = sharedRegistry.GetPosition(id);
            
            if (pos) {
                // View reads from Registry AND the MovementComponent
                raylibView.DrawMesh(id, *pos, moveComp);
            }
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
