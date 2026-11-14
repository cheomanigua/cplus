#pragma once

#include <filesystem>
#include <string_view>
#include <variant>
#include <vector>

#include "data/definitions/ammunition_definition.hpp"
#include "data/json/data_error.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

using AmmunitionCollectionDocumentLoadResult = std::variant<
    std::vector<AmmunitionDefinition>,
    JsonValidationResult
>;

using AmmunitionCollectionLoadResult = std::variant<
    std::vector<AmmunitionDefinition>,
    JsonValidationResult,
    DataError
>;

class AmmunitionCollectionLoader {
public:
    using DefinitionType = AmmunitionDefinition;

    static AmmunitionCollectionDocumentLoadResult load(
        const JsonDocument& document
    );
};

AmmunitionCollectionLoadResult loadAmmunitionCollectionJson(
    std::string_view source,
    std::string sourceName = "<memory>"
);

AmmunitionCollectionLoadResult loadAmmunitionCollectionFile(
    const std::filesystem::path& path
);

} // namespace navalsim::data

