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
                                    const ClassData& cls, const RaceData& race) {
    // 1. Entity Stats
    if (name == "Strength")     return stats.Strength;
    if (name == "Intelligence") return stats.Intelligence;
    if (name == "Health")       return stats.Health;
    if (name == "Mana")         return stats.Mana;
    
    // 2. Class Data (No cast needed, return by value)
    if (name == "ClassStr")     return static_cast<float>(cls.ClassStr);
    if (name == "ClassInt")     return static_cast<float>(cls.ClassInt);
    
    // 3. Race Data (No cast needed, return by value)
    if (name == "RaceStr")      return static_cast<float>(race.RaceStr);
    if (name == "RaceInt")      return static_cast<float>(race.RaceInt);
    
    return 0.0f;
}


void FormulaProcessor::Execute(const std::string& formulaName, EntityStats& stats, 
                               const ClassData& cls, const RaceData& race) {
    // Log the initiation of the formula execution

    // Check if the requested formula exists; return early if invalid to avoid runtime crashes
    if (_formulas.find(formulaName) == _formulas.end()) {
        return;
    }

    // Iterate through the operations defined in the JSON for this specific formula
    for (auto& op : _formulas[formulaName]["Operations"]) {
        std::string targetName = op["Target"];
        
        // 1. Retrieve the current value of the target attribute
        float currentVal = GetStatRef(targetName, stats, cls, race);
        
        // 2. Resolve the operand value (either fixed or looked up from a Source)
        float value = op.value("Value", 0.0f);
        if (op.contains("Source")) {
            std::string sourceName = op["Source"];
            value = GetStatRef(sourceName, stats, cls, race);
        }

        // 3. Perform the math operation
        std::string type = op["Type"];
        if      (type == "Set")      currentVal = value;
        else if (type == "Add")      currentVal += value;
        else if (type == "Multiply") currentVal *= value;

        // 4. Persistence: Write the result back to the specific EntityStats member
        // This acts as the "One Point of Truth" for updating the state
        if      (targetName == "Strength")     stats.Strength = currentVal;
        else if (targetName == "Intelligence") stats.Intelligence = currentVal;
        else if (targetName == "Health")       stats.Health = currentVal;
        else if (targetName == "Mana")         stats.Mana = currentVal;
        else {
        }
    }
}
