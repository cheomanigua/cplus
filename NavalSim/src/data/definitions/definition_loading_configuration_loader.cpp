#include "data/definitions/definition_loading_configuration_loader.hpp"

#include <string>
#include <utility>
#include <vector>

#include "data/json/json_object_validator.hpp"

namespace navalsim::data {

DefinitionLoadingConfigurationResult DefinitionLoadingConfigurationLoader::load(
    const JsonDocument& document
) {
    const auto& json = document.value();

    JsonObjectValidator rootValidator(json);
    rootValidator.requireArray("definition_sources");

    if (!json.is_object()) {
        return rootValidator.result();
    }

    rootValidator.allowOnlyFields({"definition_sources"});

    JsonValidationResult validation = rootValidator.result();

    if (!json.contains("definition_sources") || !json.at("definition_sources").is_array()) {
        return validation;
    }

    std::vector<DefinitionSource> sources;

    const auto& sourceArray = json.at("definition_sources");

    for (std::size_t index = 0; index < sourceArray.size(); ++index) {
        const auto& sourceJson = sourceArray.at(index);
        const std::string sourcePath = "$.definition_sources[" + std::to_string(index) + "]";

        JsonObjectValidator sourceValidator(sourceJson, sourcePath);
        sourceValidator.requireString("category", true);
        sourceValidator.requireString("directory", true);

        if (sourceJson.is_object()) {
            sourceValidator.allowOnlyFields({"category", "directory"});
        }

        const auto sourceValidation = sourceValidator.result();
        validation.append(sourceValidation);

        if (!sourceValidation.valid()) {
            continue;
        }

        sources.push_back({
            DefinitionCategory(sourceJson.at("category").get<std::string>()),
            std::filesystem::path(sourceJson.at("directory").get<std::string>())
        });
    }

    if (!validation.valid()) {
        return validation;
    }

    return DefinitionLoadingConfiguration(std::move(sources));
}

} // namespace
