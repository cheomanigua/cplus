#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/definition_loader.hpp"

namespace navalsim::data {

struct DuplicateDefinitionIdError {
    std::string id;
    std::string conflictingSource;
};

using DefinitionBatchErrorDetail = std::variant<
    DataError,
    JsonValidationResult,
    DuplicateDefinitionIdError
>;

struct DefinitionBatchError {
    std::string source;
    DefinitionBatchErrorDetail detail;
};

struct DefinitionBatchResult {
    std::size_t definitionsLoaded = 0;
    std::vector<DefinitionBatchError> errors;

    bool success() const noexcept {
        return errors.empty();
    }
};

template<typename Loader>
requires (DefinitionLoader<Loader> || DefinitionCollectionLoader<Loader>)
DefinitionBatchResult loadDefinitionFiles(
    const std::vector<std::filesystem::path>& paths,
    DefinitionDatabase& database
) {
    using Definition = typename Loader::DefinitionType;

    std::vector<Definition> stagedDefinitions;
    std::unordered_map<std::string, std::string> stagedIds;
    std::vector<DefinitionBatchError> errors;

    stagedDefinitions.reserve(paths.size());
    stagedIds.reserve(paths.size());

    for (const auto& path : paths) {
        std::vector<Definition> fileDefinitions;

        if constexpr (DefinitionLoader<Loader>) {
            auto result = loadDefinitionFile<Loader>(path);

            if (auto* error = std::get_if<DataError>(&result)) {
                errors.push_back({path.string(), std::move(*error)});
                continue;
            }

            if (auto* validation =
                    std::get_if<JsonValidationResult>(&result)) {
                errors.push_back({path.string(), std::move(*validation)});
                continue;
            }

            fileDefinitions.push_back(
                std::move(std::get<Definition>(result))
            );
        } else {
            auto result = loadDefinitionCollectionFile<Loader>(path);

            if (auto* error = std::get_if<DataError>(&result)) {
                errors.push_back({path.string(), std::move(*error)});
                continue;
            }

            if (auto* validation =
                    std::get_if<JsonValidationResult>(&result)) {
                errors.push_back({path.string(), std::move(*validation)});
                continue;
            }

            fileDefinitions =
                std::move(std::get<std::vector<Definition>>(result));
        }

        for (auto& definition : fileDefinitions) {
            const std::string id = definition.id().value();

            const auto [iterator, inserted] =
                stagedIds.try_emplace(id, path.string());

            if (!inserted) {
                errors.push_back({
                    path.string(),
                    DuplicateDefinitionIdError{id, iterator->second}
                });
                continue;
            }

            if (database.contains(definition.id())) {
                errors.push_back({
                    path.string(),
                    DuplicateDefinitionIdError{
                        id,
                        "<existing database>"
                    }
                });
                continue;
            }

            stagedDefinitions.push_back(std::move(definition));
        }
    }

    if (!errors.empty()) {
        return DefinitionBatchResult{0, std::move(errors)};
    }

    const std::size_t loadedCount = stagedDefinitions.size();
    database.addBatch(std::move(stagedDefinitions));

    return DefinitionBatchResult{loadedCount, {}};
}

} // namespace navalsim::data
