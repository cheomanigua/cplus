#pragma once
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Engine/NPCBlueprint.hpp"

// Spawns the first entity from the loader and returns the ID and blueprint
inline std::pair<int32_t, NPCBlueprint> SpawnFirstNPC(EntityRegistry& registry, DataLoader& loader) {
    loader.LoadManifest("manifest.json");
    auto templates = loader.GetNPCTemplates();
    
    // Get the first item from the templates
    const NPCBlueprint& firstBp = templates.begin()->second;
    int32_t id = registry.SpawnNPC(firstBp);
    
    return {id, firstBp};
}
