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

    const std::vector<ItemData>& GetItems() const { return _items; }
    // Renamed to GetNPCTemplates to reflect they are now blueprints/templates
    const std::vector<NPCBlueprint>& GetNPCTemplates() const { return _npcs; }

    const ClassData& GetClassData(const std::string& className) const;
    const RaceData& GetRaceData(const std::string& raceName) const;

private:
    std::vector<ItemData> _items;
    std::vector<NPCBlueprint> _npcs;

    std::unordered_map<std::string, ClassData> _classes;
    std::unordered_map<std::string, RaceData> _races;

    void LoadItemFile(const std::string& filename);
    void LoadCharacterFile(const std::string& filename);
};
