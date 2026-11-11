#pragma once

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

class JsonObjectValidator {
public:
    explicit JsonObjectValidator(const nlohmann::json& value, std::string rootPath = "$");

    void requireString(std::string_view field, bool nonEmpty = false);
    void optionalString(std::string_view field, bool nonEmpty = false);
    void requireNumber(std::string_view field, std::optional<double> minimum = std::nullopt, std::optional<double> maximum = std::nullopt);
    void requireInteger(std::string_view field);
    void requireBoolean(std::string_view field);
    void requireObject(std::string_view field);
    void requireArray(std::string_view field);

    void allowOnlyFields(std::initializer_list<std::string_view> fields);

    JsonValidationResult result() const;

private:
    const nlohmann::json& value_;
    std::string rootPath_;
    JsonValidationResult result_;

    const nlohmann::json* findField(std::string_view field, bool required);
    std::string fieldPath(std::string_view field) const;
};

} // namespace
