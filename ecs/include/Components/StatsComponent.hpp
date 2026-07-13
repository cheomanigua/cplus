#pragma once
#include <vector>
#include "Core/Constants.hpp"
#include "Core/Model.hpp"

struct StatsComponent {
    // Using a vector for contiguous memory, indexed by EntityID
    std::vector<EntityStats> Data;

    StatsComponent() {
        // Pre-allocate memory based on your engine's capacity
        Data.resize(EngineConfig::MaxEntities);
        
        // Optionally initialize base stats to a default state
        for (auto& stats : Data) {
            stats.Strength = 0.0f;
            stats.Intelligence = 0.0f;
            stats.Dexterity = 0.0f;
            stats.Health = 0.0f;
            stats.Mana = 0.0f;
            stats.IsDirty = false;
        }
    }
};
