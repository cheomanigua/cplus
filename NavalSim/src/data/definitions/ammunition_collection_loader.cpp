
#include "data/definitions/ammunition_collection_loader.hpp"

#include <limits>
#include <string>
#include <unordered_set>
#include <utility>

#include <nlohmann/json.hpp>

#include "data/json/json_object_validator.hpp"
#include "data/json/json_reader.hpp"

namespace navalsim::data {
namespace {

using Json = nlohmann::json;

void validateAmmunition(
    const Json& ammunition,
    const std::string& path,
    JsonValidationResult& result
) {
    JsonObjectValidator validator(ammunition, path);
    validator.requireString("id", true);
    validator.requireString("name", true);
    validator.requireNumber("range", std::numeric_limits<double>::min());
    validator.requireNumber("speed", std::numeric_limits<double>::min());
    validator.requireNumber("damage", std::numeric_limits<double>::min());
    validator.allowOnlyFields({
        "id",
        "name",
        "range",
        "speed",
        "is_guided",
        "damage"
    });
    result.append(validator.result());

    if (!ammunition.is_object()) return;

    const auto guidedIt = ammunition.find("is_guided");
    if (guidedIt == ammunition.end()) {
        result.addIssue(path + ".is_guided", "Required field is missing.");
    } else if (!guidedIt->is_boolean()) {
        result.addIssue(path + ".is_guided", "Expected a boolean value.");
    }
}

} // namespace

AmmunitionCollectionDocumentLoadResult AmmunitionCollectionLoader::load(
    const JsonDocument& document
) {
    const Json& json = document.value();
    JsonValidationResult validation;

    JsonObjectValidator rootValidator(json);
    rootValidator.requireInteger("schema_version");
    rootValidator.requireArray("ammunition");
    rootValidator.allowOnlyFields({"schema_version", "ammunition"});
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

    const auto ammunitionIt = json.find("ammunition");

    if (ammunitionIt != json.end() && ammunitionIt->is_array()) {
        std::unordered_set<std::string> ids;

        for (std::size_t index = 0; index < ammunitionIt->size(); ++index) {
            const Json& ammunition = ammunitionIt->at(index);
            const std::string path =
                "$.ammunition[" + std::to_string(index) + "]";

            validateAmmunition(ammunition, path, validation);

            if (ammunition.is_object()) {
                const auto idIt = ammunition.find("id");

                if (idIt != ammunition.end() && idIt->is_string()) {
                    const std::string id = idIt->get<std::string>();

                    if (!id.empty() && !ids.insert(id).second) {
                        validation.addIssue(
                            path + ".id",
                            "Duplicate ammunition ID in collection: " + id + "."
                        );
                    }
                }
            }
        }
    }

    if (!validation.valid()) return validation;

    std::vector<AmmunitionDefinition> definitions;
    definitions.reserve(ammunitionIt->size());

    for (const Json& ammunition : *ammunitionIt) {
        definitions.emplace_back(
            DefinitionId(ammunition.at("id").get<std::string>()),
            ammunition.at("name").get<std::string>(),
            ammunition.at("range").get<double>(),
            ammunition.at("speed").get<double>(),
            ammunition.at("is_guided").get<bool>(),
            ammunition.at("damage").get<double>()
        );
    }

    return definitions;
}

AmmunitionCollectionLoadResult loadAmmunitionCollectionJson(
    std::string_view source,
    std::string sourceName
) {
    auto readResult = JsonReader::parse(source, std::move(sourceName));

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = AmmunitionCollectionLoader::load(
        std::get<JsonDocument>(readResult)
    );

    if (auto* definitions =
            std::get_if<std::vector<AmmunitionDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

AmmunitionCollectionLoadResult loadAmmunitionCollectionFile(
    const std::filesystem::path& path
) {
    auto readResult = JsonReader::readFile(path);

    if (auto* error = std::get_if<DataError>(&readResult)) {
        return *error;
    }

    auto result = AmmunitionCollectionLoader::load(
        std::get<JsonDocument>(readResult)
    );

    if (auto* definitions =
            std::get_if<std::vector<AmmunitionDefinition>>(&result)) {
        return std::move(*definitions);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

} // namespace navalsim::data

