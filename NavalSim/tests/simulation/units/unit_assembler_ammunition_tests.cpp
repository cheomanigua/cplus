#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

#include <doctest/doctest.h>

#include "core/geography/geographic_types.hpp"
#include "data/definitions/ammunition_definition.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/definition_database.hpp"
#include "data/definitions/definition_id.hpp"
#include "data/definitions/unit_definition.hpp"
#include "simulation/units/component_installation_components.hpp"
#include "simulation/units/unit_assembler.hpp"
#include "world/world.hpp"

namespace
{

using navalsim::data::AmmunitionDefinition;
using navalsim::data::ComponentCategory;
using navalsim::data::ComponentDefinition;
using navalsim::data::DefinitionDatabase;
using navalsim::data::DefinitionId;
using navalsim::data::UnitComponentReference;
using navalsim::data::UnitDefinition;
using navalsim::data::UnitEnvironment;
using navalsim::geography::Elevation;
using navalsim::geography::GeographicPosition;
using navalsim::geography::Latitude;
using navalsim::geography::Longitude;
using navalsim::simulation::AmmunitionInventory;
using navalsim::simulation::ComponentInstallation;
using navalsim::simulation::UnitAssembler;
using navalsim::simulation::UnitAssemblyConfiguration;
using navalsim::simulation::ComponentInstallationLoadout;
using navalsim::simulation::AmmunitionLoadoutEntry;

GeographicPosition makePosition()
{
    return GeographicPosition(
        Latitude(35.0),
        Longitude(140.0),
        Elevation(25.0)
    );
}

DefinitionDatabase makeDefinitions()
{
    DefinitionDatabase definitions;

    definitions.add<ComponentDefinition>(
        ComponentDefinition(
            DefinitionId("naval_gun"),
            "Naval Gun",
            ComponentCategory::Simulation,
            "gun"
        )
    );

    definitions.add<AmmunitionDefinition>(
        AmmunitionDefinition(
            DefinitionId("127mm_he_shell"),
            "127mm HE Shell",
            24.0,
            800.0,
            false,
            100.0
        )
    );

    definitions.add<AmmunitionDefinition>(
        AmmunitionDefinition(
            DefinitionId("127mm_ap_shell"),
            "127mm AP Shell",
            20.0,
            850.0,
            false,
            120.0
        )
    );

    definitions.add<UnitDefinition>(
        UnitDefinition(
            DefinitionId("test_unit"),
            "Test Unit",
            "ship",
            UnitEnvironment::Water,
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("naval_gun"), 2)
            }
        )
    );

    return definitions;
}

std::vector<flecs::entity_t> getChildren(
    ::World& world,
    flecs::entity_t parentId
)
{
    std::vector<flecs::entity_t> children;

    world.ecs().entity(parentId).children(
        [&children](flecs::entity child)
        {
            children.push_back(child.id());
        }
    );

    return children;
}

std::optional<flecs::entity_t> findInstallationId(
    ::World& world,
    const std::vector<flecs::entity_t>& children,
    std::size_t installationIndex
)
{
    for (const auto childId : children)
    {
        const auto child = world.ecs().entity(childId);
        const auto installation = child.get<ComponentInstallation>();

        if (installation.installationIndex == installationIndex)
        {
            return childId;
        }
    }

    return std::nullopt;
}

} // namespace

TEST_CASE("UnitAssembler assigns ammunition to individual installations")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {
                    AmmunitionLoadoutEntry{
                        DefinitionId("127mm_he_shell"),
                        200
                    }
                }
            },
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                1,
                {}
            }
        }
    };

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition(),
        0.0,
        configuration
    );

    const auto children = getChildren(world, unitId);
    REQUIRE(children.size() == 2);

    const auto firstId = findInstallationId(world, children, 0);
    const auto secondId = findInstallationId(world, children, 1);

    REQUIRE(firstId.has_value());
    REQUIRE(secondId.has_value());

    const auto first = world.ecs().entity(*firstId);
    const auto second = world.ecs().entity(*secondId);

    REQUIRE(first.has<AmmunitionInventory>());
    REQUIRE(second.has<AmmunitionInventory>());

    const auto firstInventory = first.get<AmmunitionInventory>();
    const auto secondInventory = second.get<AmmunitionInventory>();

    REQUIRE(firstInventory.entries.size() == 1);
    CHECK(
        firstInventory.entries[0].ammunitionDefinitionId
        == "127mm_he_shell"
    );
    CHECK(firstInventory.entries[0].quantity == 200);

    CHECK(secondInventory.entries.empty());
}

TEST_CASE("UnitAssembler supports multiple ammunition types per installation")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {
                    AmmunitionLoadoutEntry{
                        DefinitionId("127mm_he_shell"),
                        200
                    },
                    AmmunitionLoadoutEntry{
                        DefinitionId("127mm_ap_shell"),
                        50
                    }
                }
            }
        }
    };

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition(),
        0.0,
        configuration
    );

    const auto children = getChildren(world, unitId);
    REQUIRE(children.size() == 2);

    const auto firstId = findInstallationId(world, children, 0);
    const auto secondId = findInstallationId(world, children, 1);

    REQUIRE(firstId.has_value());
    REQUIRE(secondId.has_value());

    const auto first = world.ecs().entity(*firstId);
    const auto second = world.ecs().entity(*secondId);

    REQUIRE(first.has<AmmunitionInventory>());
    CHECK_FALSE(second.has<AmmunitionInventory>());

    const auto inventory = first.get<AmmunitionInventory>();

    REQUIRE(inventory.entries.size() == 2);
    CHECK(
        inventory.entries[0].ammunitionDefinitionId
        == "127mm_he_shell"
    );
    CHECK(inventory.entries[0].quantity == 200);
    CHECK(
        inventory.entries[1].ammunitionDefinitionId
        == "127mm_ap_shell"
    );
    CHECK(inventory.entries[1].quantity == 50);
}

TEST_CASE("UnitAssembler rejects unknown ammunition definitions")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {
                    AmmunitionLoadoutEntry{
                        DefinitionId("unknown_ammunition"),
                        10
                    }
                }
            }
        }
    };

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("test_unit"),
            makePosition(),
            0.0,
            configuration
        ),
        std::invalid_argument
    );
}

TEST_CASE("UnitAssembler rejects zero ammunition quantities")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {
                    AmmunitionLoadoutEntry{
                        DefinitionId("127mm_he_shell"),
                        0
                    }
                }
            }
        }
    };

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("test_unit"),
            makePosition(),
            0.0,
            configuration
        ),
        std::invalid_argument
    );
}

TEST_CASE("UnitAssembler rejects loadouts for unknown installations")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                2,
                {}
            }
        }
    };

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("test_unit"),
            makePosition(),
            0.0,
            configuration
        ),
        std::invalid_argument
    );
}

TEST_CASE("UnitAssembler rejects duplicate loadouts for an installation")
{
    const auto definitions = makeDefinitions();
    ::World world;
    UnitAssembler assembler;

    const UnitAssemblyConfiguration configuration{
        .componentInstallations = {
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {}
            },
            ComponentInstallationLoadout{
                DefinitionId("naval_gun"),
                0,
                {}
            }
        }
    };

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("test_unit"),
            makePosition(),
            0.0,
            configuration
        ),
        std::invalid_argument
    );
}

