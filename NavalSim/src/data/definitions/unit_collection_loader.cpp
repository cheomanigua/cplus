#include "data/definitions/unit_collection_loader.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

#include <nlohmann/json.hpp>

#include "data/json/json_object_validator.hpp"
#include "data/json/json_reader.hpp"

namespace navalsim::data {
namespace {

using Json = nlohmann::json;

bool isPositiveInteger(const Json& value) {
    if (!value.is_number_integer()) return false;

    if (value.is_number_unsigned()) {
        const auto quantity = value.get<std::uint64_t>();
        return quantity > 0 &&
            quantity <= std::numeric_limits<std::size_t>::max();
    }

    const auto quantity = value.get<std::int64_t>();
    return quantity > 0 &&
        static_cast<std::uint64_t>(quantity) <=
            std::numeric_limits<std::size_t>::max();
}

void validateFuel(
    const Json& fuel,
    const std::string& path,
    JsonValidationResult& result
) {
    if (fuel.is_null()) return;

    if (!fuel.is_object()) {
        result.addIssue(path, "Fuel must be an object or null.");
        return;
    }

    JsonObjectValidator validator(fuel, path);
    validator.requireNumber("capacity", std::numeric_limits<double>::min());
    validator.requireNumber(
        "reference_speed_knots",
        std::numeric_limits<double>::min()
    );
    validator.requireNumber(
        "consumption_per_hour",
        std::numeric_limits<double>::min()
    );
    validator.allowOnlyFields({
        "capacity",
        "reference_speed_knots",
        "consumption_per_hour"
    });
    result.append(validator.result());
}

void validateUnit(
    const Json& unit,
    const std::string& path,
    JsonValidationResult& result
) {
    JsonObjectValidator validator(unit, path);
    validator.requireString("id", true);
    validator.requireString("name", true);
    validator.requireString("type", true);
    validator.requireString("environment", true);
    validator.requireNumber("max_speed_knots", 0.0);
    validator.requireArray("components");
    validator.allowOnlyFields({
        "id",
        "name",
        "type",
        "environment",
        "max_speed_knots",
        "fuel",
        "components"
    });
    result.append(validator.result());

    if (!unit.is_object()) return;

    const auto fuelIt = unit.find("fuel");
    if (fuelIt != unit.end()) {
        validateFuel(*fuelIt, path + ".fuel", result);
    }

    const auto componentsIt = unit.find("components");
    if (componentsIt != unit.end() && componentsIt->is_array()) {
        for (std::size_t index = 0; index < componentsIt->size(); ++index) {
            const Json& component = componentsIt->at(index);
            const std::string componentPath =
                path + ".components[" + std::to_string(index) + "]";

            JsonObjectValidator componentValidator(component, componentPath);
            componentValidator.requireString("component_id", true);
            componentValidator.requireInteger("quantity");
            componentValidator.allowOnlyFields({"component_id", "quantity"});
            result.append(componentValidator.result());

            if (component.is_object()) {
                const auto quantityIt = component.find("quantity");
                if (quantityIt != component.end() &&
                    quantityIt->is_number_integer() &&
                    !isPositiveInteger(*quantityIt)) {
                    result.addIssue(
                        componentPath + ".quantity",
                        "Quantity must be a positive integer."
                    );
                }
            }
        }
    }

    const auto environmentIt = unit.find("environment");
    if (environmentIt != unit.end() && environmentIt->is_string()) {
        const std::string environment =
            environmentIt->get<std::string>();

        if (environment != "water" &&
            environment != "air" &&
            environment != "land") {
            result.addIssue(
                path + ".environment",
                "Environment must be 'water', 'air', or 'land'."
            );
        }
    }
}

UnitEnvironment parseEnvironment(const std::string& environment) {
    if (environment == "water") return UnitEnvironment::Water;
    if (environment == "air") return UnitEnvironment::Air;
    return UnitEnvironment::Land;
}

std::optional<FuelProperties> parseFuel(const Json& unit) {
    const auto fuelIt = unit.find("fuel");

    if (fuelIt == unit.end() || fuelIt->is_null()) {
        return std::nullopt;
    }

    return FuelProperties{
        fuelIt->at("capacity").get<double>(),
        fuelIt->at("reference_speed_knots").get<double>(),
        fuelIt->at("consumption_per_hour").get<double>()
    };
}

} // namespace

UnitCollectionDocumentLoadResult UnitCollectionLoader::load(
    const JsonDocument& document
) {
    const Json& json = document.value();
    JsonValidationResult validation;

    JsonObjectValidator rootValidator(json);
    rootValidator.requireInteger("schema_version");
    rootValidator.requireArray("units");
    rootValidator.allowOnlyFields({"schema_version", "units"});
    validation.append(rootValidator.result());

    if (!json.is_object()) return validation;

    const auto versionIt = json.find("schema_version");

    if (versionIt != json.end() &&
        versionIt->is_number_integer() &&
        versionIt->get<long double>() != 1.0L) {
        validation.addIssue(
            "$.schema_version",
            "Unsupported schema version; expected 1."
        );
    }

    const auto unitsIt = json.find("units");

    if (unitsIt != json.end() && unitsIt->is_array()) {
        std::unordered_set<std::string> ids;

        for (std::size_t index = 0; index < unitsIt->size(); ++index) {
            const Json& unit = unitsIt->at(index);
            const std::string path =
                "$.units[" + std::to_string(index) + "]";

            validateUnit(unit, path, validation);

            if (unit.is_object()) {
                const auto idIt = unit.find("id");

                if (idIt != unit.end() && idIt->is_string()) {
                    const std::string id = idIt->get<std::string>();

                    if (!id.empty() && !ids.insert(id).second) {
                        validation.addIssue(
                            path + ".id",
                            "Duplicate unit ID in collection: " + id + "."
                        );
                    }
                }
            }
        }
    }

    if (!validation.valid()) return validation;

    std::vector<UnitDefinition> definitions;
    definitions.reserve(unitsIt->size());

    for (const Json& unit : *unitsIt) {
        std::vector<UnitComponentReference> components;
        components.reserve(unit.at("components").size());

        for (const Json& component : unit.at("components")) {
            components.emplace_back(
                DefinitionId(
                    component.at("component_id").get<std::string>()
                ),
                component.at("quantity").get<std::size_t>()
            );
        }

        definitions.emplace_back(
            DefinitionId(unit.at("id").get<std::string>()),
            unit.at("name").get<std::string>(),
            unit.at("type").get<std::string>(),
            parseEnvironment(unit.at("environment").get<std::string>()),
            unit.at("max_speed_knots").get<double>(),
            parseFuel(unit),
            std::move(components)
        );
    }

    return definitions;
}

UnitCollectionLoadResult loadUnitCollectionJson(
    std::string_view source,
    std::string sourceName
) {
    auto readResult = JsonReader::parse(source, std::move(sourceName));

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = UnitCollectionLoader::load(
        std::get<JsonDocument>(readResult)
    );

    if (auto* definitions =
            std::get_if<std::vector<UnitDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

UnitCollectionLoadResult loadUnitCollectionFile(
    const std::filesystem::path& path
) {
    auto readResult = JsonReader::readFile(path);

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = UnitCollectionLoader::load(
        std::get<JsonDocument>(readResult)
    );

    if (auto* definitions =
            std::get_if<std::vector<UnitDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

} // namespace navalsim::data
