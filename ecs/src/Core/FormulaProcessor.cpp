#include "Core/FormulaProcessor.hpp"
#include <fstream>
#include <iostream>

std::unordered_map<std::string, nlohmann::json> FormulaProcessor::_formulas;

void FormulaProcessor::Initialize(const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    nlohmann::json j;
    file >> j;
    for (auto& [name, formula] : j.items()) {
        _formulas[name] = formula;
    }
}

float FormulaProcessor::GetStatRef(const std::string& name, EntityStats& stats, 
                                    const ClassData& cls, const RaceData& race,
                                    const ItemEffectContext& context) {

    // 1. Check Item Context first (e.g., Damage from Dagger, Healing from Potion)
    if (name == "BonusValue") return context.BonusValue;

    // 2. Entity Stats
    if (name == "Strength")     return stats.Strength;
    if (name == "Intelligence") return stats.Intelligence;
    if (name == "Dexterity")    return stats.Dexterity;
    if (name == "Health")       return stats.Health;
    if (name == "Mana")         return stats.Mana;
    
    // 3. Class Data (No cast needed, return by value)
    if (name == "ClassHealth")  return static_cast<float>(cls.ClassHealth);
    if (name == "ClassMana")    return static_cast<float>(cls.ClassMana);
    if (name == "ClassStr")     return static_cast<float>(cls.ClassStr);
    if (name == "ClassInt")     return static_cast<float>(cls.ClassInt);
    if (name == "ClassDex")     return static_cast<float>(cls.ClassDex);
    
    // 4. Race Data (No cast needed, return by value)
    if (name == "RaceStr")      return static_cast<float>(race.RaceStr);
    if (name == "RaceInt")      return static_cast<float>(race.RaceInt);
    if (name == "RaceDex")      return static_cast<float>(race.RaceDex);
    
    return 0.0f;
}

// PERSISTENT: Modifies EntityStats directly
// PERSISTENT: UpdateStats does not use items, so we pass an empty context
void FormulaProcessor::ExecuteUpdateStats(const std::string& formulaName, EntityStats& stats, 
                                          const ClassData& cls, const RaceData& race) {
    if (_formulas.find(formulaName) == _formulas.end()) return;

    ItemEffectContext emptyContext; // Dummy context for stat updates

    for (auto& op : _formulas[formulaName]["Operations"]) {
        std::string targetName = op["Stat"];
        float currentVal = GetStatRef(targetName, stats, cls, race, emptyContext);
        
        float value = op.value("Value", 0.0f);
        if (op.contains("Source")) {
            value = GetStatRef(op["Source"], stats, cls, race, emptyContext);
        }

        // FIX: Declare 'type' before using it!
        std::string type = op["Type"]; 

        if      (type == "Set")      currentVal = value;
        else if (type == "Add")      currentVal += value;
        else if (type == "Multiply") currentVal *= value;

        // Apply back to state
        if      (targetName == "Strength")     stats.Strength = currentVal;
        else if (targetName == "Intelligence") stats.Intelligence = currentVal;
        else if (targetName == "Dexterity")    stats.Dexterity = currentVal;
        else if (targetName == "Health")       stats.Health = currentVal;
        else if (targetName == "Mana")         stats.Mana = currentVal;
    }
}

// TRANSIENT: Returns a result, no side effects
// TRANSIENT: Combat Actions use the real ItemEffectContext
float FormulaProcessor::ExecuteAction(const std::string& formulaName, const EntityStats& stats, 
                                      const ClassData& cls, const RaceData& race,
                                      const ItemEffectContext& context) {

    if (_formulas.find(formulaName) == _formulas.end()) return 0.0f;

    float result = 0.0f;
    for (auto& op : _formulas[formulaName]["Operations"]) {
        float val = op.value("Value", 0.0f);

        // Use the passed context to resolve sources like "BonusValue"
        if (op.contains("Source")) {
            val = GetStatRef(op["Source"], const_cast<EntityStats&>(stats), cls, race, context);
        }

        std::string type = op["Type"];
        if      (type == "Set")      result = val;
        else if (type == "Add")      result += val;
        else if (type == "Multiply") result *= val;
    }
    return result;
}
