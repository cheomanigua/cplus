#include "Engine/DataLoader.hpp"
#include "Core/Constants.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DataLoader::DataLoader(const std::string& dataDirectory) 
    : _dataDirectory(dataDirectory) {
    _items.reserve(EngineConfig::MaxItemCapacity);
}

const NPCBlueprint* DataLoader::GetBlueprintById(int id) const {
    for (const auto& npc : _npcs) {
        if (npc.EntityId == id) return &npc;
    }
    return nullptr;
}


bool DataLoader::LoadManifest(const std::string& manifestFilename) {
    std::ifstream file(_dataDirectory + "/" + manifestFilename);
    if (!file.is_open()) return false;

    json manifest = json::parse(file);
    for (const auto& modulePath : manifest["ConfigModules"]) {
        std::string path = modulePath.get<std::string>();
        if (path.find("items/") != std::string::npos) {
            LoadItemFile(path);
        } else {
            LoadCharacterFile(path);
        }
    }
    return true;
}

void DataLoader::LoadItemFile(const std::string& filename) {
    std::ifstream file(_dataDirectory + "/" + filename);
    if (!file.is_open()) return;
    
    json data = json::parse(file);

    for (auto it = data.begin(); it != data.end(); ++it) {
        ItemData item;
        item.Id = std::stoi(it.key());
        
        auto& details = it.value();
        item.Name = details["Name"].get<std::string>();
        
        if (details.contains("Slot")) {
            item.Slot = details["Slot"].get<std::string>();
        }
        
        if (details.contains("GrantedComponents")) {
            item.GrantedComponents = details["GrantedComponents"].get<std::vector<GrantedComponent>>();
        }
        
        _items.push_back(std::move(item));
    }
}

void DataLoader::LoadCharacterFile(const std::string& filename) {
    std::ifstream file(_dataDirectory + "/" + filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return;
    }

    json data = json::parse(file);

    if (filename.find("npc_blueprint.json") != std::string::npos) {
        _npcs = data.get<std::vector<NPCBlueprint>>();
        std::cout << "Loaded " << _npcs.size() << " NPCs." << std::endl;
    } 
    else if (filename.find("classes.json") != std::string::npos) {
        // Populates the map for GetClassData lookups
        _classes = data.get<std::unordered_map<std::string, ClassData>>();
    } 
    else if (filename.find("races.json") != std::string::npos) {
        // Populates the map for GetRaceData lookups
        _races = data.get<std::unordered_map<std::string, RaceData>>();
    }
}

const ClassData& DataLoader::GetClassData(const std::string& className) const {
    // If your data isn't loaded, this will throw an exception. 
    // Ensure these maps are populated during LoadManifest!
    return _classes.at(className);
}

const RaceData& DataLoader::GetRaceData(const std::string& raceName) const {
    return _races.at(raceName);
}
