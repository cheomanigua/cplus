#pragma once

#include <variant>

#include "data/definitions/definition_loading_configuration.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

using DefinitionLoadingConfigurationResult = std::variant<
    DefinitionLoadingConfiguration,
    JsonValidationResult
>;

class DefinitionLoadingConfigurationLoader {
public:
    static DefinitionLoadingConfigurationResult load(const JsonDocument& document);
};

}
