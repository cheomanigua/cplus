#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"

#include "Engine/EngineDriver.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/ConsoleGameView.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/PathResolver.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Core/Commands/GameCommand.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/SpatialSystem.hpp"
#include "Tests/TestRunner.hpp"
#include <iostream>


int main() {
    TestRunner::RunAll();

    // Initialize the DataLoader
    DataLoader loader;
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    
    // Initialize FormulaProcessor
    FormulaProcessor::Initialize(Engine::GetDataPath("system/formulas.json"));

    // Create the SHARED Registry
    EntityRegistry sharedRegistry(loader.GetItems());

    // Setup Views
    ConsoleGameView consoleView; 

    // Declare components BEFORE engine drivers
    StatsComponent statsComp;
    PositionComponent posComp;
    MovementComponent moveComp;
    
    // Initialize Engines
    EngineDriver consoleEngine(&consoleView, loader, loader.GetItems(), "/data", &sharedRegistry, statsComp, posComp, moveComp);
    
    InitWindow(800, 600, "Data Driven Engine");
    SetTargetFPS(60);
    RaylibGameView raylibView;
    raylibView.SetRegistry(&sharedRegistry);
    EngineFacade::Implementation = &raylibView;
    EngineDriver graphicsEngine(&raylibView, loader, loader.GetItems(), "/data", &sharedRegistry, statsComp, posComp, moveComp);
    SpatialSystem spatialSystem;

    // SPAWN NPCs (Factory Pattern)
    // We no longer use loader.GetNPCs() to get IDs; we use templates to spawn entities.
    const auto& npcTemplates = loader.GetNPCTemplates(); 
    std::cout << "Spawning " << npcTemplates.size() << " NPCs via factory..." << std::endl;
    
    // Change the loop to use structured bindings for key-value pairs
    for (const auto& [name, templateData] : npcTemplates) {
        // 'name' is the key (e.g., "Thrall"), 'templateData' is the NPCBlueprint struct
        
        // The Registry handles _nextId++ and returns the unique ID for us
        int32_t newId = sharedRegistry.SpawnNPC(templateData, statsComp);
        // IMPORTANT: Manually sync the position from the template to the component
        posComp.Positions[newId] = templateData.SpawnPosition;

        // Use the returned ID to command the engine logic
        consoleEngine.AddCommand(GameCommand{ CommandType::UpdateStats, newId });
    }

    // Run engine tick
    consoleEngine.Tick(0.0f);

    consoleView.DisplayFullCharacterSheet(sharedRegistry, posComp, statsComp, loader);

    // Main Loop
    while (!WindowShouldClose()) {
        // 1. Maintain Spatial State via the System
        spatialSystem.Update(sharedRegistry, posComp);
    
        // 2. InputSystem handles Selection AND Movement Commands
        // No longer calling raylibView.GetNextCommand() here!
        InputSystem::PollInput(graphicsEngine.GetCommandQueue(), sharedRegistry, posComp, spatialSystem);
        
        // 3. Engine updates logic
        graphicsEngine.Tick(GetFrameTime());
        
        // 4. Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Access movement component from the engine to pass to DrawMesh
        const auto& moveComp = graphicsEngine.GetMovementComponent();
        
        for (int32_t id : sharedRegistry.GetActiveEntities()) {
            raylibView.DrawMesh(id, posComp.Positions[id], moveComp);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
