#pragma once

#include <filesystem>
#include <string_view>
#include <variant>
#include <vector>

#include "data/definitions/component_definition.hpp"
#include "data/json/data_error.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

using ComponentCollectionDocumentLoadResult = std::variant<
    std::vector<ComponentDefinition>,
    JsonValidationResult
>;

using ComponentCollectionLoadResult = std::variant<
    std::vector<ComponentDefinition>,
    JsonValidationResult,
    DataError
>;

class ComponentCollectionLoader {
public:
    using DefinitionType = ComponentDefinition;

    static ComponentCollectionDocumentLoadResult load(
        const JsonDocument& document
    );
};

ComponentCollectionLoadResult loadComponentCollectionJson(
    std::string_view source,
    std::string sourceName = "<memory>"
);

ComponentCollectionLoadResult loadComponentCollectionFile(
    const std::filesystem::path& path
);

} // namespace navalsim::data
