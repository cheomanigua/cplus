#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace navalsim::simulation
{

enum class ComponentCondition
{
    Operational,
    Damaged,
    Destroyed
};

struct ComponentInstallation
{
    std::string definitionId;
    std::size_t installationIndex = 0;
};

struct AmmunitionInventoryEntry
{
    std::string ammunitionDefinitionId;
    std::size_t quantity;
};

struct AmmunitionInventory
{
    std::vector<AmmunitionInventoryEntry> entries;
};

struct RadarConfiguration
{
    double rangeNm;
};

enum class RadarOperationalState
{
    Enabled,
    Disabled
};

} // namespace navalsim::simulation

