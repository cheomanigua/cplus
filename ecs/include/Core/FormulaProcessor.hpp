#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

struct Operation {
    std::string Type;
    std::string Stat;
    std::string Target;
    std::string Source;
    float Value;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Operation, Type, Stat, Target, Source, Value)

struct Formula {
    std::vector<Operation> Operations;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Formula, Operations)

class FormulaProcessor {
public:
    static void Initialize(const std::string& jsonFilePath);
    
    // Bind entity-specific data to string names for the formula to read
    static void RegisterSource(const std::string& name, std::function<float()> getter);
    
    static float Execute(const std::string& formulaName, std::vector<float>& stats);

private:
    static float ResolveSource(const std::string& source);
    static std::unordered_map<std::string, Formula> _formulas;
    static std::unordered_map<std::string, std::function<float()>> _sourceMap;
};
