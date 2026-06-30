#include "Engine/EngineDriver.hpp"
#include "Core/ConsoleGameView.hpp"

#include <iostream>
#include "Engine/DataLoader.hpp"

int main() {
    // 1. Initialize the DataLoader with the path to your data directory
    DataLoader loader("data"); 

    // 2. Load the manifest (which triggers loading of npc_blueprints.json)
    if (!loader.LoadManifest("manifest.json")) {
        std::cerr << "Failed to load manifest!" << std::endl;
        return -1;
    }
    std::cout << "Successfully loaded " << loader.GetItems().size() << " items." << std::endl;

    // 3. Retrieve and print NPC data
    const auto& npcs = loader.GetNPCs();
    
    std::cout << "Successfully loaded " << npcs.size() << " NPCs:" << std::endl;
    for (const auto& npc : npcs) {
        std::cout << "---------------------------" << std::endl;
        std::cout << "ID: " << npc.EntityId << std::endl;
        std::cout << "Name: " << npc.Name << std::endl;
        std::cout << "Class: " << npc.Class << std::endl;
        std::cout << "Race: " << npc.Race << std::endl;
        std::cout << "Spawn: (" << npc.SpawnPosition.X << ", " 
                  << npc.SpawnPosition.Y << ")" << std::endl;
    }

    return 0;
}
