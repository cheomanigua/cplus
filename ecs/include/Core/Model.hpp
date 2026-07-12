#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>
#include "Core/Constants.hpp"

// Helper to allow std::optional with nlohmann::json
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt) j = *opt; else j = nullptr;
        }
        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) opt = std::nullopt;
            else opt = j.get<T>();
        }
    };
}

// Forward declaration of EntityStats
struct EntityStats {
    float Strength;
    float Intelligence;
    float Dexterity;
    float Health;
    float Mana;
    bool IsDirty;

    // Add this method to expose your stats for the FormulaProcessor
    std::unordered_map<std::string, float> GetDynamicAttributes() const {
        return {
            {"Strength", Strength},
            {"Intelligence", Intelligence},
            {"Dexterity", Dexterity},
            {"Health", Health},
            {"Mana", Mana}
            // Add any other stats you need to register
        };
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityStats, Strength, Intelligence, Dexterity, Health, Mana, IsDirty)

// --- 1. Character Data Records ---
struct RaceData {
    int RaceStr;
    int RaceInt;
    int RaceDex;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RaceData, RaceStr, RaceInt, RaceDex)

struct ClassData {
    int ClassStr;
    int ClassInt;
    int ClassDex;
    int ClassHealth;
    int ClassMana;
    std::string ClassPrimarySkill;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClassData, ClassStr, ClassInt, ClassDex, ClassHealth, ClassMana, ClassPrimarySkill)

struct SkillData {
    std::string AttributeScale;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SkillData, AttributeScale)

// --- 2. Context for FormulaProcessor ---
struct FormulaContext {
    const EntityStats* Stats; 
    std::optional<ClassData> Class;
    std::optional<RaceData> Race;
    int WeaponDamage = 0;
    std::map<std::string, float> ExtraParams;
};

// --- 3. Attribute/Modifier Records ---
struct ModifierDto {
    std::string Stat;
    float Value;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ModifierDto, Stat, Value)

// --- 4. Component Data Structures ---
struct GrantedComponent {
    std::string Tag;
    std::optional<std::vector<ModifierDto>> Modifiers;
    std::map<std::string, std::string> Properties;

    // Manual deserialization to handle missing keys
    friend void from_json(const nlohmann::json& j, GrantedComponent& gc) {
        j.at("Tag").get_to(gc.Tag);
        
        // Only load Modifiers if the key actually exists
        if (j.contains("Modifiers") && !j["Modifiers"].is_null()) {
            gc.Modifiers = j.at("Modifiers").get<std::vector<ModifierDto>>();
        } else {
            gc.Modifiers = std::nullopt;
        }

        j.at("Properties").get_to(gc.Properties);
    }
};

// --- 5. Main Item Record ---
struct ItemData {
    int Id;
    std::string Name;
    std::optional<std::string> Slot;
    std::vector<GrantedComponent> GrantedComponents;
};
// Note: ItemData is loaded manually in DataLoader.cpp due to ID keys in JSON
