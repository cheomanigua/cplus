#ifndef NPC_BLUEPRINT_HPP
#define NPC_BLUEPRINT_HPP

#include <string>
#include <nlohmann/json.hpp>

// Define Vector2 to match System.Numerics.Vector2
struct Vector2 { 
    float X; 
    float Y; 
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, X, Y)

struct NPCBlueprint {
    int EntityId;
    std::string Name;
    std::string Race;
    std::string Class;
    int EquippedItemId;
    Vector2 SpawnPosition;
};

// The macro MUST contain every field you want to load from JSON
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NPCBlueprint, EntityId, Name, Race, Class, EquippedItemId, SpawnPosition)

#endif
