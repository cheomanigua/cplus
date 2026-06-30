#include "Core/FormulaProcessor.hpp"
#include <fstream>

std::unordered_map<std::string, Formula> FormulaProcessor::_formulas;
std::unordered_map<std::string, std::function<float()>> FormulaProcessor::_sourceMap;

void FormulaProcessor::Initialize(const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) return;
    nlohmann::json j;
    file >> j;
    _formulas = j.get<std::unordered_map<std::string, Formula>>();
}

void FormulaProcessor::RegisterSource(const std::string& name, std::function<float()> getter) {
    _sourceMap[name] = getter;
}

float FormulaProcessor::ResolveSource(const std::string& source) {
    auto it = _sourceMap.find(source);
    return (it != _sourceMap.end()) ? it->second() : 0.0f;
}


float FormulaProcessor::Execute(const std::string& formulaName, const std::string& targetStat) {
    float result = 0.0f;
    
    // Only process operations that match the target stat we want to calculate
    for (const auto& op : _formulas[formulaName].Operations) {
        if (op.Target != targetStat) continue; 

        float input = !op.Source.empty() ? ResolveSource(op.Source) : op.Value;
        
        if (op.Type == "Set") result = input;
        else if (op.Type == "Add") result += input;
        else if (op.Type == "Multiply") result *= input;
    }
    return result;
}

void from_json(const nlohmann::json& j, Operation& op) {
    j.at("Type").get_to(op.Type);
    
    // Use .value() to provide defaults if the key is missing
    op.Stat = j.value("Stat", "");
    op.Target = j.value("Target", "");
    op.Source = j.value("Source", "");
    op.Value = j.value("Value", 0.0f);
}
