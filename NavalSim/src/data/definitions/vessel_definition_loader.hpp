#pragma once

#include <variant>

#include "data/definitions/vessel_definition.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

using VesselDefinitionLoadResult = std::variant<VesselDefinition, JsonValidationResult>;

class VesselDefinitionLoader {
public:
    using DefinitionType = VesselDefinition;

    static VesselDefinitionLoadResult load(const JsonDocument& document);
};

}
