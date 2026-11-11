#include "data/definitions/vessel_definition_loader.hpp"

#include <limits>
#include <string>

#include "data/json/json_object_validator.hpp"

namespace navalsim::data {

VesselDefinitionLoadResult VesselDefinitionLoader::load(const JsonDocument& document) {
    const auto& json = document.value();

    JsonObjectValidator rootValidator(json);
    rootValidator.requireString("id", true);
    rootValidator.requireString("name", true);
    rootValidator.requireObject("dimensions");
    rootValidator.requireObject("propulsion");

    if (!json.is_object()) {
        return rootValidator.result();
    }

    rootValidator.allowOnlyFields({"id", "name", "dimensions", "propulsion"});
    JsonValidationResult validation = rootValidator.result();

    if (json.contains("dimensions") && json.at("dimensions").is_object()) {
        JsonObjectValidator dimensionsValidator(json.at("dimensions"), "$.dimensions");

        dimensionsValidator.requireNumber("length_m", std::numeric_limits<double>::min());
        dimensionsValidator.requireNumber("beam_m", std::numeric_limits<double>::min());
        dimensionsValidator.requireNumber("draft_m", std::numeric_limits<double>::min());
        dimensionsValidator.allowOnlyFields({"length_m", "beam_m", "draft_m"});

        validation.append(dimensionsValidator.result());
    }

    if (json.contains("propulsion") && json.at("propulsion").is_object()) {
        JsonObjectValidator propulsionValidator(json.at("propulsion"), "$.propulsion");

        propulsionValidator.requireNumber("maximum_speed_mps", std::numeric_limits<double>::min());
        propulsionValidator.allowOnlyFields({"maximum_speed_mps"});

        validation.append(propulsionValidator.result());
    }

    if (!validation.valid()) {
        return validation;
    }

    const auto& dimensions = json.at("dimensions");
    const auto& propulsion = json.at("propulsion");

    VesselDefinition definition(
        DefinitionId(json.at("id").get<std::string>()),
        json.at("name").get<std::string>(),
        VesselDimensions(
            geography::Distance(dimensions.at("length_m").get<double>()),
            geography::Distance(dimensions.at("beam_m").get<double>()),
            geography::Distance(dimensions.at("draft_m").get<double>())
        ),
        VesselPropulsion(propulsion.at("maximum_speed_mps").get<double>())
    );

    return definition;
}

} // namespace
