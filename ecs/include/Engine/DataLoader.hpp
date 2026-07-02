#pragma once
#include <string>
#include <vector>
#include <unordered_map> // Added for std::unordered_map
#include "Engine/NPCBlueprint.hpp"
#include "Core/Model.hpp"

class DataLoader {
public:
    // Removed dataDirectory parameter as it is no longer needed
    DataLoader() = default; 

    // Updated parameter name to 'filename' for consistency
    bool LoadManifest(const std::string& filename);

    const std::vector<ItemData>& GetItems() const { return _items; }
    const std::vector<NPCBlueprint>& GetNPCs() const { return _npcs; }

    const NPCBlueprint* GetBlueprintById(int id) const;
    const ClassData& GetClassData(const std::string& className) const;
    const RaceData& GetRaceData(const std::string& raceName) const;

private:
    // Removed _dataDirectory member variable
    std::vector<ItemData> _items;
    std::vector<NPCBlueprint> _npcs;

    std::unordered_map<std::string, ClassData> _classes;
    std::unordered_map<std::string, RaceData> _races;

    // Standardized parameter name to 'filename'
    void LoadItemFile(const std::string& filename);
    void LoadCharacterFile(const std::string& filename);
};
