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

float FormulaProcessor::Execute(const std::string& formulaName, std::vector<float>& stats) {
    if (_formulas.find(formulaName) == _formulas.end()) return 0.0f;
    
    float result = 0.0f;
    for (const auto& op : _formulas[formulaName].Operations) {
        float input = !op.Source.empty() ? ResolveSource(op.Source) : op.Value;
        
        if (op.Type == "Add") result += input;
        else if (op.Type == "Multiply") result *= input;
        else if (op.Type == "Set") result = input;
    }
    return result;
}
