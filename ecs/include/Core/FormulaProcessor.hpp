#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Core/Model.hpp"

class FormulaProcessor {
public:
    static void Initialize(const std::string& jsonFilePath);
    static void Execute(const std::string& formulaName, EntityStats& stats, 
                        const ClassData& cls, const RaceData& race);

private:
    static std::unordered_map<std::string, nlohmann::json> _formulas;
    
    // The "Single Point of Truth" for attribute names.
    // Update this function when adding new attributes.
    static float GetStatRef(const std::string& name, EntityStats& stats, 
                             const ClassData& cls, const RaceData& race);
};
