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
    float Value = 0.0f;
};

void from_json(const nlohmann::json& j, Operation& op);

struct Formula {
    std::vector<Operation> Operations;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Formula, Operations)

class FormulaProcessor {
public:
    static void Initialize(const std::string& jsonFilePath);
    
    // Bind entity-specific data to string names for the formula to read
    static void RegisterSource(const std::string& name, std::function<float()> getter);
    
    static float Execute(const std::string& formulaName, const std::string& targetStat);

private:
    static float ResolveSource(const std::string& source);
    static std::unordered_map<std::string, Formula> _formulas;
    static std::unordered_map<std::string, std::function<float()>> _sourceMap;
};
