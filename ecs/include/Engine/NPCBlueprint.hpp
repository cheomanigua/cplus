#ifndef NPC_BLUEPRINT_HPP
#define NPC_BLUEPRINT_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "raylib.h" // Raylib provides the native Vector2

// You no longer define struct Vector2 here. 
// Raylib provides it for you!

struct NPCBlueprint {
    int EntityId;
    std::string Name;
    std::string Race;
    std::string Class;
    int EquippedItemId;
    Vector2 SpawnPosition; // Uses Raylib's Vector2
};

// nlohmann::json needs to know how to handle Raylib's Vector2.
// Since it's a POD struct (float x, y), you can define a simple mapping:
namespace nlohmann {
    template <>
    struct adl_serializer<Vector2> {
        static void from_json(const json& j, Vector2& v) {
            v.x = j.at("X").get<float>();
            v.y = j.at("Y").get<float>();
        }
    };
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NPCBlueprint, EntityId, Name, Race, Class, EquippedItemId, SpawnPosition)

#endif
