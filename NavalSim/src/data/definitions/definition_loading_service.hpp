#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "data/definitions/definition_batch_loader.hpp"
#include "data/definitions/definition_category.hpp"
#include "data/definitions/definition_loader_registry.hpp"
#include "data/definitions/definition_loading_configuration.hpp"

namespace navalsim::data {

enum class DefinitionSourceErrorCode {
    UnknownCategory,
    DirectoryNotFound,
    DirectoryScanFailed
};

struct DefinitionSourceError {
    DefinitionSourceErrorCode code;
    std::string category;
    std::filesystem::path directory;
    std::string message;
};

struct DefinitionCategoryLoadResult {
    DefinitionCategory category;
    std::size_t filesDiscovered;
    DefinitionRegistryLoadResult result;
};

struct DefinitionLoadingReport {
    std::vector<DefinitionSourceError> sourceErrors;
    std::vector<DefinitionCategoryLoadResult> categoryResults;

    bool success() const noexcept {
        if (!sourceErrors.empty()) {
            return false;
        }

        for (const auto& categoryResult : categoryResults) {
            const auto* batchResult = std::get_if<DefinitionBatchResult>(&categoryResult.result);

            if (batchResult == nullptr || !batchResult->success()) {
                return false;
            }
        }

        return true;
    }
};

DefinitionLoadingReport loadConfiguredDefinitions(
    const DefinitionLoadingConfiguration& configuration,
    const std::filesystem::path& contentRoot,
    const DefinitionLoaderRegistry& registry,
    DefinitionDatabase& database
);

} // namespace
