#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Engine/NPCBlueprint.hpp"
#include "Core/Model.hpp"

class DataLoader {
public:
    DataLoader() = default; 

    bool LoadManifest(const std::string& filename);

    // Getters now return the Map instead of Vector
    const std::unordered_map<int32_t, ItemData>& GetItems() const { return _items; }
    const std::unordered_map<std::string, NPCBlueprint>& GetNPCTemplates() const { return _npcs; }

    const ClassData& GetClassData(const std::string& className) const;
    const RaceData& GetRaceData(const std::string& raceName) const;

    // Optional: Add a helper to get a single blueprint by name
    const NPCBlueprint* GetNPCBlueprint(const std::string& name) const {
        auto it = _npcs.find(name);
        return (it != _npcs.end()) ? &it->second : nullptr;
    }

private:
    // Changed to unordered_maps for O(1) lookup and order-independence
    std::unordered_map<int32_t, ItemData> _items;
    std::unordered_map<std::string, NPCBlueprint> _npcs;

    std::unordered_map<std::string, ClassData> _classes;
    std::unordered_map<std::string, RaceData> _races;

    void LoadItemFile(const std::string& filename);
    void LoadCharacterFile(const std::string& filename);
};
