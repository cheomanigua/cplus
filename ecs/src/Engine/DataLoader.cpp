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
        std::cout << "Failed to open file!" << std::endl;
        return;
    }

    json data = json::parse(file);
    if (filename.find("npc_blueprint.json") != std::string::npos) {
        _npcs = data.get<std::vector<NPCBlueprint>>();
        std::cout << "Loaded " << _npcs.size() << " NPCs from " << filename << std::endl;
    }
}
