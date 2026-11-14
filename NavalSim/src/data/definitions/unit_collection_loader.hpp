#pragma once

#include <filesystem>
#include <string_view>
#include <variant>
#include <vector>

#include "data/definitions/unit_definition.hpp"
#include "data/json/data_error.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

using UnitCollectionDocumentLoadResult = std::variant<
    std::vector<UnitDefinition>,
    JsonValidationResult
>;

using UnitCollectionLoadResult = std::variant<
    std::vector<UnitDefinition>,
    JsonValidationResult,
    DataError
>;

class UnitCollectionLoader {
public:
    using DefinitionType = UnitDefinition;

    static UnitCollectionDocumentLoadResult load(const JsonDocument& document);
};

UnitCollectionLoadResult loadUnitCollectionJson(
    std::string_view source,
    std::string sourceName = "<memory>"
);

UnitCollectionLoadResult loadUnitCollectionFile(
    const std::filesystem::path& path
);

} // namespace navalsim::data
