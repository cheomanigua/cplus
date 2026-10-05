#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "Core/Model.hpp"

struct EntityMetadata {
    std::string Name;
    std::string Class;
    std::string Race;
};

struct IdentityComponent {
    // We use a vector sized to MaxEntities to allow direct index access by entityID
    std::vector<EntityMetadata> Metadata;

    IdentityComponent(size_t maxEntities) {
        Metadata.resize(maxEntities);
    }
};
