#pragma once
#include <string>
#include <vector>
#include "Engine/NPCBlueprint.hpp"
#include "Core/Model.hpp"

class DataLoader {
public:
    DataLoader(const std::string& dataDirectory);
    bool LoadManifest(const std::string& manifestFilename);

    const std::vector<ItemData>& GetItems() const { return _items; }
    const std::vector<NPCBlueprint>& GetNPCs() const { return _npcs; }

    const NPCBlueprint* GetBlueprintById(int id) const;
    const ClassData& GetClassData(const std::string& className) const;
    const RaceData& GetRaceData(const std::string& raceName) const;

private:
    std::string _dataDirectory;
    std::vector<ItemData> _items;
    std::vector<NPCBlueprint> _npcs;

    std::unordered_map<std::string, ClassData> _classes;
    std::unordered_map<std::string, RaceData> _races;

    void LoadItemFile(const std::string& filename);
    void LoadCharacterFile(const std::string& filename);
};
