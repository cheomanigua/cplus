#include "simulation/units/unit_assembler.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "data/definitions/ammunition_definition.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/definition_database.hpp"
#include "data/definitions/unit_definition.hpp"
#include "domains/fuel/fuel_components.hpp"
#include "domains/movement/movement_components.hpp"
#include "simulation/units/component_installation_components.hpp"

namespace navalsim::simulation
{
namespace
{

constexpr double KNOTS_TO_METRES_PER_SECOND = 1852.0 / 3600.0;

using InstallationKey = std::pair<std::string, std::size_t>;

struct ResolvedAmmunition
{
    const navalsim::data::AmmunitionDefinition* definition;
    std::size_t quantity;
};

struct ResolvedInstallation
{
    const navalsim::data::ComponentDefinition* definition;
    std::size_t installationIndex;
    bool hasAmmunitionLoadout = false;
    std::vector<ResolvedAmmunition> ammunition;
};

std::vector<ResolvedInstallation> resolveInstallations(
    const navalsim::data::DefinitionDatabase& definitions,
    const navalsim::data::UnitDefinition& unitDefinition,
    const UnitAssemblyConfiguration& configuration
)
{
    std::vector<ResolvedInstallation> installations;
    std::map<std::string, std::size_t> nextInstallationIndex;

    for (const auto& componentReference : unitDefinition.components())
    {
        const auto& componentId = componentReference.componentId();

        const auto* definition =
            definitions.find<navalsim::data::ComponentDefinition>(
                componentId
            );

        if (definition == nullptr)
        {
            throw std::invalid_argument(
                "Unknown component definition: " + componentId.value()
            );
        }

        for (std::size_t i = 0; i < componentReference.quantity(); ++i)
        {
            const std::size_t installationIndex =
                nextInstallationIndex[componentId.value()]++;

            installations.push_back(
                ResolvedInstallation{
                    definition,
                    installationIndex
                }
            );
        }
    }

    std::set<InstallationKey> configuredInstallations;

    for (const auto& loadout : configuration.componentInstallations)
    {
        const InstallationKey key{
            loadout.componentDefinitionId.value(),
            loadout.installationIndex
        };

        if (!configuredInstallations.insert(key).second)
        {
            throw std::invalid_argument(
                "Duplicate ammunition loadout for component installation: " +
                key.first + "[" + std::to_string(key.second) + "]"
            );
        }

        auto installation = installations.end();

        for (auto iterator = installations.begin();
             iterator != installations.end();
             ++iterator)
        {
            if (iterator->definition->id() == loadout.componentDefinitionId &&
                iterator->installationIndex == loadout.installationIndex)
            {
                installation = iterator;
                break;
            }
        }

        if (installation == installations.end())
        {
            throw std::invalid_argument(
                "Unknown component installation for ammunition loadout: " +
                key.first + "[" + std::to_string(key.second) + "]"
            );
        }

        installation->hasAmmunitionLoadout = true;
        installation->ammunition.reserve(loadout.ammunition.size());

        for (const auto& ammunitionEntry : loadout.ammunition)
        {
            if (ammunitionEntry.quantity == 0)
            {
                throw std::invalid_argument(
                    "Ammunition quantity must be greater than zero: " +
                    ammunitionEntry.ammunitionId.value()
                );
            }

            const auto* ammunitionDefinition =
                definitions.find<navalsim::data::AmmunitionDefinition>(
                    ammunitionEntry.ammunitionId
                );

            if (ammunitionDefinition == nullptr)
            {
                throw std::invalid_argument(
                    "Unknown ammunition definition: " +
                    ammunitionEntry.ammunitionId.value()
                );
            }

            installation->ammunition.push_back(
                ResolvedAmmunition{
                    ammunitionDefinition,
                    ammunitionEntry.quantity
                }
            );
        }
    }

    return installations;
}

void installComponent(
    flecs::world& ecs,
    flecs::entity_t unitId,
    const ResolvedInstallation& resolvedInstallation
)
{
    const auto& definition = *resolvedInstallation.definition;

    auto installation = ecs.entity();
    installation.child_of(unitId);

    installation.set<ComponentInstallation>(
        ComponentInstallation{
            definition.id().value(),
            resolvedInstallation.installationIndex
        }
    );

    installation.set<ComponentCondition>(
        ComponentCondition::Operational
    );

    if (resolvedInstallation.hasAmmunitionLoadout)
    {
        AmmunitionInventory inventory;
        inventory.entries.reserve(
            resolvedInstallation.ammunition.size()
        );

        for (const auto& ammunition : resolvedInstallation.ammunition)
        {
            inventory.entries.push_back(
                AmmunitionInventoryEntry{
                    ammunition.definition->id().value(),
                    ammunition.quantity
                }
            );
        }

        installation.set<AmmunitionInventory>(std::move(inventory));
    }

    if (definition.type() == "radar")
    {
        const auto& radarProperties =
            std::get<navalsim::data::RadarProperties>(
                *definition.properties()
            );

        installation.set<RadarConfiguration>(
            RadarConfiguration{radarProperties.rangeNm()}
        );

        installation.set<RadarOperationalState>(
            RadarOperationalState::Enabled
        );
    }
}

} // namespace

flecs::entity_t UnitAssembler::assemble(
    const navalsim::data::DefinitionDatabase& definitions,
    World& world,
    const navalsim::data::DefinitionId& definitionId,
    navalsim::geography::GeographicPosition position,
    double initialHeadingDegrees,
    const UnitAssemblyConfiguration& configuration
) const
{
    const auto* unitDefinition =
        definitions.find<navalsim::data::UnitDefinition>(definitionId);

    if (unitDefinition == nullptr)
    {
        throw std::invalid_argument(
            "Unknown unit definition: " + definitionId.value()
        );
    }

    // Resolve and validate every installation and ammunition reference
    // before creating any runtime entities.
    const auto installations =
        resolveInstallations(
            definitions,
            *unitDefinition,
            configuration
        );

    const double maximumSpeedMetresPerSecond =
        unitDefinition->maxSpeedKnots() * KNOTS_TO_METRES_PER_SECOND;

    auto& ecs = world.ecs();
    auto unit = ecs.entity();

    try
    {
        unit.set<navalsim::geography::GeographicPosition>(position);

        unit.set<navalsim::movement::Heading>(
            navalsim::movement::Heading{initialHeadingDegrees}
        );

        unit.set<navalsim::movement::DesiredHeading>(
            navalsim::movement::DesiredHeading{initialHeadingDegrees}
        );

        unit.set<navalsim::movement::Speed>(
            navalsim::movement::Speed{0.0}
        );

        unit.set<navalsim::movement::MaximumSpeed>(
            navalsim::movement::MaximumSpeed{
                maximumSpeedMetresPerSecond
            }
        );

        unit.set<navalsim::movement::TurnRate>(
            navalsim::movement::TurnRate{0.0}
        );

        unit.set<navalsim::movement::TurnDirection>(
            navalsim::movement::TurnDirection::None
        );

        unit.set<navalsim::movement::MovementStatus>(
            navalsim::movement::MovementStatus::Stopped
        );

        if (unitDefinition->fuel().has_value())
        {
            const auto& fuel = *unitDefinition->fuel();

            unit.set<navalsim::fuel::FuelState>(
                navalsim::fuel::FuelState{
                    fuel.capacity,
                    fuel.capacity,
                    fuel.referenceSpeedKnots,
                    fuel.consumptionPerHour
                }
            );
        }

        for (const auto& installation : installations)
        {
            installComponent(
                ecs,
                unit.id(),
                installation
            );
        }
    }
    catch (...)
    {
        unit.destruct();
        throw;
    }

    return unit.id();
}

} // namespace navalsim::simulation

