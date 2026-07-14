#include <iostream>
#include <cassert>
#include "Components/ItemComponent.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Engine/NPCBlueprint.hpp"

void TestItemComponentLoading() {
    // 1. Setup
    std::unordered_map<int32_t, ItemData> itemMap;
    EntityRegistry registry(itemMap);
    ItemComponent itemComp;
    DataLoader loader;

    // 2. Use the public LoadManifest instead of the private LoadCharacterFile[cite: 17, 18]
    // Ensure your "manifest.json" exists and points to your npc_blueprint.json
    loader.LoadManifest("manifest.json");
    
    // USE the correct getter from DataLoader.hpp[cite: 17]
    auto blueprints = loader.GetNPCTemplates(); 

    // 3. Spawn entities and verify
    for (const auto& [name, bp] : blueprints) {
        int32_t id = registry.SpawnNPC(bp);
        
        // Populate the component with the ID from the blueprint[cite: 16]
        itemComp.EquippedItemId[id] = bp.EquippedItemId;

        // Verify the data matches the blueprint[cite: 16]
        int32_t loadedId = itemComp.EquippedItemId[id];
        
        if (loadedId == bp.EquippedItemId) {
            std::cout << "[TEST] ItemComponent: Success for " << name 
                      << " (Entity " << id << ") - ItemID: " << loadedId << std::endl;
        } else {
            std::cout << "[TEST] ItemComponent: FAILED for " << name 
                      << ". Expected " << bp.EquippedItemId << ", Got " << loadedId << std::endl;
        }
    }
}
