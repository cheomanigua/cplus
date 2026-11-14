
#include "data/definitions/component_collection_loader.hpp"

#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "data/definitions/component_definition.hpp"
#include "data/json/json_object_validator.hpp"
#include "data/json/json_reader.hpp"

namespace navalsim::data {
namespace {

void validateComponent(
    const nlohmann::json& json,
    const std::string& path,
    std::vector<ComponentDefinition>& definitions,
    JsonValidationResult& result)
{
    JsonObjectValidator validator(json, path);

    validator.requireString("id", true);
    validator.requireString("name", true);
    validator.requireString("category", true);
    validator.requireString("type", true);

    validator.allowOnlyFields({
        "id",
        "name",
        "category",
        "type",
        "properties"
    });

    auto componentResult = validator.result();

    if (!componentResult.valid()) {
        result.append(componentResult);
        return;
    }

    const std::string id = json["id"].get<std::string>();
    const std::string name = json["name"].get<std::string>();
    const std::string category = json["category"].get<std::string>();
    const std::string type = json["type"].get<std::string>();

    if (category != "damage_only" && category != "simulation") {
        result.addIssue(
            path + ".category",
            "Category must be either 'damage_only' or 'simulation'.");
        return;
    }

    if (type == "radar" && category != "simulation") {
        result.addIssue(
            path + ".category",
            "Radar components must use the simulation category.");
        return;
    }

    const bool hasProperties = json.contains("properties");

    if (category == "damage_only") {
        if (hasProperties) {
            result.addIssue(
                path + ".properties",
                "Damage-only components must not define properties.");
            return;
        }

        definitions.emplace_back(
            DefinitionId(id),
            name,
            ComponentCategory::DamageOnly,
            type);

        return;
    }

    // Simulation component types other than radar do not yet have
    // a defined technical properties schema.
    if (type != "radar") {
        if (hasProperties) {
            result.addIssue(
                path + ".properties",
                "Properties are not yet supported for this simulation component type.");
            return;
        }

        definitions.emplace_back(
            DefinitionId(id),
            name,
            ComponentCategory::Simulation,
            type);

        return;
    }

    // Radar components require a properties object.
    if (!hasProperties) {
        result.addIssue(
            path + ".properties",
            "Radar components must define properties.");
        return;
    }

    if (!json["properties"].is_object()) {
        result.addIssue(
            path + ".properties",
            "Expected a JSON object.");
        return;
    }

    JsonObjectValidator propertiesValidator(
        json["properties"],
        path + ".properties");

    propertiesValidator.requireNumber(
        "range_nm",
        std::numeric_limits<double>::min());

    propertiesValidator.allowOnlyFields({"range_nm"});

    const auto propertiesResult = propertiesValidator.result();

    if (!propertiesResult.valid()) {
        result.append(propertiesResult);
        return;
    }

    const double rangeNm = json["properties"]["range_nm"].get<double>();

    definitions.emplace_back(
        DefinitionId(id),
        name,
        ComponentCategory::Simulation,
        type,
        ComponentProperties{RadarProperties(rangeNm)});
}

} // namespace

ComponentCollectionDocumentLoadResult ComponentCollectionLoader::load(
    const JsonDocument& document)
{
    const auto& json = document.value();

    JsonObjectValidator validator(json, "$");

    validator.requireInteger("schema_version");
    validator.requireArray("components");

    validator.allowOnlyFields({
        "schema_version",
        "components"
    });

    JsonValidationResult result = validator.result();

    if (!result.valid()) {
        return result;
    }

    const int schemaVersion = json["schema_version"].get<int>();

    if (schemaVersion != 1) {
        result.addIssue(
            "$.schema_version",
            "Unsupported schema version.");
        return result;
    }

    const auto& components = json["components"];

    std::vector<ComponentDefinition> definitions;
    definitions.reserve(components.size());

    for (std::size_t i = 0; i < components.size(); ++i) {
        const std::string path =
            "$.components[" + std::to_string(i) + "]";

        validateComponent(
            components[i],
            path,
            definitions,
            result);
    }

    if (!result.valid()) {
        return result;
    }

    // Reject duplicate component IDs within this collection.
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        for (std::size_t j = i + 1; j < definitions.size(); ++j) {
            if (definitions[i].id() == definitions[j].id()) {
                result.addIssue(
                    "$.components",
                    "Duplicate component definition ID '" +
                        definitions[i].id().value() + "'.");
            }
        }
    }

    if (!result.valid()) {
        return result;
    }

    return definitions;
}

ComponentCollectionLoadResult loadComponentCollectionJson(
    std::string_view source,
    std::string sourceName)
{
    auto readResult = JsonReader::parse(
        source,
        std::move(sourceName));

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = ComponentCollectionLoader::load(
        std::get<JsonDocument>(readResult));

    if (auto* definitions =
            std::get_if<std::vector<ComponentDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

ComponentCollectionLoadResult loadComponentCollectionFile(
    const std::filesystem::path& path)
{
    auto readResult = JsonReader::readFile(path);

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = ComponentCollectionLoader::load(
        std::get<JsonDocument>(readResult));

    if (auto* definitions =
            std::get_if<std::vector<ComponentDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

} // namespace navalsim::data

