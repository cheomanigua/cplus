#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "data/definitions/definition_category.hpp"

namespace navalsim::data {

struct DefinitionSource {
    DefinitionCategory category;
    std::filesystem::path directory;
};

class DefinitionLoadingConfiguration {
public:
    explicit DefinitionLoadingConfiguration(std::vector<DefinitionSource> sources)
        : sources_(std::move(sources)) {}

    const std::vector<DefinitionSource>& sources() const noexcept {
        return sources_;
    }

private:
    std::vector<DefinitionSource> sources_;
};

}
