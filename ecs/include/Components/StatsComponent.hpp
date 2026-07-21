#pragma once
#include <vector>
#include "Core/Constants.hpp"
#include "Core/Model.hpp"

struct StatsComponent {
    // Stores entity stats inline, indexed by EntityID
    std::array<EntityStats, EngineConfig::MaxEntities> Data{};

    // Encapsulated initialization logic for specific entity stats[cite: 8]
    inline void InitializeStats(int32_t id, float strength, float intelligence, float dexterity, float charisma, float health, float mana) {
        auto& stats = Data[id];
        stats.Strength = strength;
        stats.Intelligence = intelligence;
        stats.Dexterity = dexterity;
        stats.Charisma = charisma;
        stats.Health = health;
        stats.Mana = mana;
        stats.IsDirty = true; // Mark as dirty so the FormulaProcessor picks it up
    }
};
