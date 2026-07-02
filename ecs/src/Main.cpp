#include "Engine/EngineDriver.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Core/ConsoleGameView.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/FormulaProcessor.hpp"
#include <iostream>

int main() {
    // 1. Initialize the DataLoader with the path to your data directory
    DataLoader loader("data"); 

    // 2. Load the manifest
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    std::cout << "Successfully loaded " << loader.GetItems().size() << " items." << std::endl;
    
    // 3. Initialize the FormulaProcessor 
    // This loads the math logic from formulas.json into memory
    FormulaProcessor::Initialize("data/system/formulas.json");

    // Add debug print here
    std::cout << "DEBUG: Checking Class lookup..." << std::endl;
    try {
        const auto& warrior = loader.GetClassData("Warrior");
        std::cout << "Found Warrior: Str=" << warrior.ClassStr << std::endl;
    } catch (...) {
        std::cout << "Warrior data missing from map!" << std::endl;
    }

    // 4. Setup View and Engine
    ConsoleGameView view; 
    
    // Inject the loader reference so the EngineDriver can perform 
    // blueprint lookups during command execution
    EngineDriver engine(&view, loader, loader.GetItems(), "data");

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
    std::cout << "\n--- Final Computed NPC Stats ---" << std::endl;
    for (const auto& npc : npcs) {
        // Retrieve the mutable stats from the registry
        EntityStats* stats = engine.GetRegistry()->GetEntityStats(npc.EntityId);
        
        std::cout << "---------------------------" << std::endl;
        std::cout << "ID:    " << npc.EntityId << std::endl;
        std::cout << "Name:  " << npc.Name << std::endl;
        std::cout << "Class: " << npc.Class << std::endl;
        std::cout << "Race:  " << npc.Race << std::endl;
        std::cout << "Spawn: (" << npc.SpawnPosition.x << ", " 
          << npc.SpawnPosition.y << ")" << std::endl;
        
        if (stats) {
            std::cout << "Computed Stats:" << std::endl;
            std::cout << "  Strength:     " << stats->Strength << std::endl;
            std::cout << "  Intelligence: " << stats->Intelligence << std::endl;
            std::cout << "  Health:       " << stats->Health << std::endl;
            std::cout << "  Mana:         " << stats->Mana << std::endl;
        } else {
            std::cout << "  [!] Error: Stats not found for ID " << npc.EntityId << std::endl;
        }
    }

    return 0;
}
