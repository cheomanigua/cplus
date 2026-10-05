#include "Engine/DataLoader.hpp"
#include "Core/Constants.hpp"
#include "Core/PathResolver.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool DataLoader::LoadManifest(const std::string& filename) {
    std::string fullPath = Engine::GetDataPath(filename);
    std::ifstream file(fullPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open manifest: " << fullPath << std::endl;
        return false;
    }

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
    std::string fullPath = Engine::GetDataPath(filename);
    std::ifstream file(fullPath);
    if (!file.is_open()) return;

    json data = json::parse(file);
    for (auto it = data.begin(); it != data.end(); ++it) {
        int32_t id = std::stoi(it.key());
        ItemData item;
        item.Id = id;
        
        auto& details = it.value();
        item.Name = details["Name"].get<std::string>();
        if (details.contains("Slot")) item.Slot = details["Slot"].get<std::string>();
        if (details.contains("GrantedComponents")) 
            item.GrantedComponents = details["GrantedComponents"].get<std::vector<GrantedComponent>>();
        
        _items[id] = std::move(item); // Key-based storage
    }
}

void DataLoader::LoadCharacterFile(const std::string& filename) {
    std::string fullPath = Engine::GetDataPath(filename);
    std::ifstream file(fullPath);
    if (!file.is_open()) return;

    json data = json::parse(file);

    if (filename.find("npc_blueprint.json") != std::string::npos) {
        for (const auto& item : data) {
            NPCBlueprint blueprint = item.get<NPCBlueprint>();
            _npcs[blueprint.Name] = blueprint; // Key-based storage
        }
    } 
    else if (filename.find("classes.json") != std::string::npos) {
        _classes = data.get<std::unordered_map<std::string, ClassData>>();
    } 
    else if (filename.find("races.json") != std::string::npos) {
        _races = data.get<std::unordered_map<std::string, RaceData>>();
    }
}

const ClassData& DataLoader::GetClassData(const std::string& className) const {
    return _classes.at(className);
}

const RaceData& DataLoader::GetRaceData(const std::string& raceName) const {
    return _races.at(raceName);
}
