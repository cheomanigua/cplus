#include "data/definitions/definition_loading_service.hpp"

#include <algorithm>
#include <system_error>
#include <utility>
#include <vector>

namespace navalsim::data {
namespace {

struct CategorySources {
    DefinitionCategory category;
    std::vector<std::filesystem::path> files;
    bool failed = false;
};

std::filesystem::path resolveDirectory(
    const std::filesystem::path& directory,
    const std::filesystem::path& contentRoot
) {
    return directory.is_absolute() ? directory : contentRoot / directory;
}

void addSourceError(
    DefinitionLoadingReport& report,
    DefinitionSourceErrorCode code,
    const DefinitionSource& source,
    const std::filesystem::path& resolvedDirectory,
    std::string message
) {
    report.sourceErrors.push_back({
        code,
        source.category.value(),
        resolvedDirectory,
        std::move(message)
    });
}

bool discoverJsonFiles(
    const DefinitionSource& source,
    const std::filesystem::path& directory,
    CategorySources& categorySources,
    DefinitionLoadingReport& report
) {
    std::error_code error;

    if (!std::filesystem::exists(directory, error)) {
        addSourceError(
            report,
            error ? DefinitionSourceErrorCode::DirectoryScanFailed : DefinitionSourceErrorCode::DirectoryNotFound,
            source,
            directory,
            error ? "Unable to check whether the definition directory exists: " + error.message()
                  : "Definition directory does not exist."
        );
        return false;
    }

    if (!std::filesystem::is_directory(directory, error)) {
        addSourceError(
            report,
            error ? DefinitionSourceErrorCode::DirectoryScanFailed : DefinitionSourceErrorCode::DirectoryNotFound,
            source,
            directory,
            error ? "Unable to inspect the definition directory: " + error.message()
                  : "Definition source path is not a directory."
        );
        return false;
    }

    std::filesystem::directory_iterator iterator(directory, error);
    const std::filesystem::directory_iterator end;

    if (error) {
        addSourceError(
            report,
            DefinitionSourceErrorCode::DirectoryScanFailed,
            source,
            directory,
            "Unable to scan definition directory: " + error.message()
        );
        return false;
    }

    while (iterator != end) {
        const auto entry = *iterator;

        std::error_code entryError;
        if (entry.is_regular_file(entryError) && !entryError && entry.path().extension() == ".json") {
            categorySources.files.push_back(entry.path());
        } else if (entryError) {
            addSourceError(
                report,
                DefinitionSourceErrorCode::DirectoryScanFailed,
                source,
                directory,
                "Unable to inspect directory entry: " + entryError.message()
            );
            return false;
        }

        iterator.increment(error);

        if (error) {
            addSourceError(
                report,
                DefinitionSourceErrorCode::DirectoryScanFailed,
                source,
                directory,
                "Error while scanning definition directory: " + error.message()
            );
            return false;
        }
    }

    return true;
}

}

DefinitionLoadingReport loadConfiguredDefinitions(
    const DefinitionLoadingConfiguration& configuration,
    const std::filesystem::path& contentRoot,
    const DefinitionLoaderRegistry& registry,
    DefinitionDatabase& database
) {
    DefinitionLoadingReport report;
    std::vector<CategorySources> categories;

    for (const auto& source : configuration.sources()) {
        auto categoryIterator = std::find_if(
            categories.begin(),
            categories.end(),
            [&source](const CategorySources& category) {
                return category.category == source.category;
            }
        );

        if (categoryIterator == categories.end()) {
            categories.push_back({source.category, {}, false});
            categoryIterator = std::prev(categories.end());
        }

        if (!registry.isRegistered(source.category)) {
            addSourceError(
                report,
                DefinitionSourceErrorCode::UnknownCategory,
                source,
                resolveDirectory(source.directory, contentRoot),
                "No definition loader is registered for this category."
            );
            categoryIterator->failed = true;
            continue;
        }

        const auto directory = resolveDirectory(source.directory, contentRoot);

        if (!discoverJsonFiles(source, directory, *categoryIterator, report)) {
            categoryIterator->failed = true;
        }
    }

    for (auto& category : categories) {
        if (category.failed) {
            continue;
        }

        std::sort(category.files.begin(), category.files.end());

        const std::size_t filesDiscovered = category.files.size();

        auto result = registry.loadFiles(
            category.category,
            category.files,
            database
        );

        report.categoryResults.push_back({
            category.category,
            filesDiscovered,
            std::move(result)
        });
    }

    return report;
}

} // namespace
