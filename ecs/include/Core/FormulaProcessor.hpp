#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Core/Model.hpp"

// Structure to pass item-specific data (from MagicComponent) into the formula
struct ItemEffectContext {
    float BonusValue = 0.0f;
    std::string DamageType = "None";
    // Add additional properties here as needed
};

class FormulaProcessor {
public:
    static void Initialize(const std::string& jsonFilePath);
    
    // Updates stats permanently (for leveling/attribute changes)
    static void ExecuteUpdateStats(const std::string& formulaName, EntityStats& stats, 
                                   const ClassData& cls, const RaceData& race);
                                   
    // Executes transient actions (combat, healing) using item context
    static float ExecuteAction(const std::string& formulaName, const EntityStats& stats, 
                               const ClassData& cls, const RaceData& race,
                               const ItemEffectContext& context);

private:
    static std::unordered_map<std::string, nlohmann::json> _formulas;
    //
    // The "Single Point of Truth" for attribute names.
    // Update this function when adding new attributes.
    static float GetStatRef(const std::string& name, EntityStats& stats, 
                             const ClassData& cls, const RaceData& race,
                             const ItemEffectContext& context);
};
