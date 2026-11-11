
#pragma once

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "data/definitions/definition_batch_loader.hpp"
#include "data/definitions/definition_category.hpp"
#include "data/definitions/definition_loader.hpp"

namespace navalsim::data {

struct DefinitionRegistryError {
    std::string category;
    std::string message;
};

using DefinitionRegistryLoadResult = std::variant<
    DefinitionBatchResult,
    DefinitionRegistryError
>;

class DefinitionLoaderRegistry {
public:
    template<DefinitionLoader Loader>
    void registerLoader(DefinitionCategory category) {
        const std::string categoryName = category.value();

        const bool inserted = loaders_.try_emplace(
            categoryName,
            Entry{
                std::type_index(typeid(typename Loader::DefinitionType)),
                &loadFilesWithLoader<Loader>
            }
        ).second;

        if (!inserted) {
            throw std::invalid_argument(
                "Definition category is already registered: " + categoryName
            );
        }
    }

    bool isRegistered(const DefinitionCategory& category) const {
        return loaders_.contains(category.value());
    }

    std::vector<DefinitionCategory> categories() const {
        std::vector<DefinitionCategory> result;
        result.reserve(loaders_.size());

        for (const auto& loader : loaders_) {
            result.emplace_back(loader.first);
        }

        std::sort(
            result.begin(),
            result.end(),
            [](const auto& left, const auto& right) {
                return left.value() < right.value();
            }
        );

        return result;
    }

    DefinitionRegistryLoadResult loadFiles(
        const DefinitionCategory& category,
        const std::vector<std::filesystem::path>& paths,
        DefinitionDatabase& database
    ) const {
        const auto iterator = loaders_.find(category.value());

        if (iterator == loaders_.end()) {
            return DefinitionRegistryError{
                category.value(),
                "No definition loader is registered for this category."
            };
        }

        return iterator->second.loadFiles(paths, database);
    }

private:
    using BatchLoaderFunction = DefinitionBatchResult (*)(
        const std::vector<std::filesystem::path>&,
        DefinitionDatabase&
    );

    struct Entry {
        std::type_index definitionType;
        BatchLoaderFunction loadFiles;
    };

    template<DefinitionLoader Loader>
    static DefinitionBatchResult loadFilesWithLoader(
        const std::vector<std::filesystem::path>& paths,
        DefinitionDatabase& database
    ) {
        return loadDefinitionFiles<Loader>(paths, database);
    }

    std::unordered_map<std::string, Entry> loaders_;
};

} // namespace
