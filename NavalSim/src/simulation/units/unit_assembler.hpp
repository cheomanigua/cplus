#pragma once

#include <cstddef>
#include <vector>

#include "core/geography/geographic_types.hpp"
#include "data/definitions/definition_id.hpp"
#include "world/world.hpp"

namespace navalsim::data
{
class DefinitionDatabase;
}

namespace navalsim::simulation
{

struct AmmunitionLoadoutEntry
{
    navalsim::data::DefinitionId ammunitionId;
    std::size_t quantity;
};

struct ComponentInstallationLoadout
{
    navalsim::data::DefinitionId componentDefinitionId;
    std::size_t installationIndex;
    std::vector<AmmunitionLoadoutEntry> ammunition;
};

struct UnitAssemblyConfiguration
{
    std::vector<ComponentInstallationLoadout> componentInstallations;
};

class UnitAssembler
{
public:
    flecs::entity_t assemble(
        const navalsim::data::DefinitionDatabase& definitions,
        World& world,
        const navalsim::data::DefinitionId& definitionId,
        navalsim::geography::GeographicPosition position,
        double initialHeadingDegrees = 0.0,
        const UnitAssemblyConfiguration& configuration = {}
    ) const;
};

} // namespace navalsim::simulation

