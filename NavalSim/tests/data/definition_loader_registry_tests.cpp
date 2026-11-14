
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "data/definitions/definition_loader_registry.hpp"
#include "data/definitions/ammunition_collection_loader.hpp"
#include "data/definitions/ammunition_definition.hpp"
#include "data/definitions/component_collection_loader.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/unit_collection_loader.hpp"
#include "data/definitions/unit_definition.hpp"
#include "data/json/json_object_validator.hpp"

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        path_ = std::filesystem::temp_directory_path() / "navalsim_definition_registry_tests";
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::filesystem::remove_all(path_);
    }

    std::filesystem::path writeFile(
        const std::string& filename,
        const std::string& contents
    ) const {
        const auto path = path_ / filename;
        std::ofstream file(path);
        file << contents;
        return path;
    }

private:
    std::filesystem::path path_;
};

class AircraftTestDefinition {
public:
    explicit AircraftTestDefinition(navalsim::data::DefinitionId id)
        : id_(std::move(id)) {}

    const navalsim::data::DefinitionId& id() const noexcept {
        return id_;
    }

private:
    navalsim::data::DefinitionId id_;
};

class AircraftTestLoader {
public:
    using DefinitionType = AircraftTestDefinition;

    static std::variant<
        DefinitionType,
        navalsim::data::JsonValidationResult
    > load(const navalsim::data::JsonDocument& document) {
        navalsim::data::JsonObjectValidator validator(document.value());
        validator.requireString("id", true);
        validator.allowOnlyFields({"id"});

        auto validation = validator.result();
        if (!validation.valid()) {
            return validation;
        }

        return DefinitionType(
            navalsim::data::DefinitionId(
                document.value().at("id").get<std::string>()
            )
        );
    }
};

}

TEST_CASE("Definition category rejects empty identifiers") {
    CHECK_THROWS_AS(
        navalsim::data::DefinitionCategory(""),
        std::invalid_argument
    );
}

TEST_CASE("Definition loader registry discovers registered categories") {
    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<AircraftTestLoader>(
        navalsim::data::DefinitionCategory("aircraft")
    );

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
    );

    CHECK(registry.isRegistered(
        navalsim::data::DefinitionCategory("aircraft")
    ));

    CHECK(registry.isRegistered(
        navalsim::data::DefinitionCategory("units")
    ));

    CHECK_FALSE(registry.isRegistered(
        navalsim::data::DefinitionCategory("weapon")
    ));

    const auto categories = registry.categories();

    REQUIRE(categories.size() == 2);
    CHECK(categories[0].value() == "aircraft");
    CHECK(categories[1].value() == "units");
}

TEST_CASE("Definition loader registry rejects duplicate category registrations") {
    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<AircraftTestLoader>(
        navalsim::data::DefinitionCategory("aircraft")
    );

    CHECK_THROWS_AS(
        registry.registerLoader<navalsim::data::UnitCollectionLoader>(
            navalsim::data::DefinitionCategory("aircraft")
        ),
        std::invalid_argument
    );

    CHECK(registry.categories().size() == 1);
}

TEST_CASE("Definition loader registry dispatches loading by category") {
    TemporaryDirectory directory;

    const auto aircraftPath = directory.writeFile(
        "aircraft.json",
        R"({"id": "test-aircraft"})"
    );

    const auto unitsPath = directory.writeFile(
        "units.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "test-vessel",
                    "name": "Test Vessel",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 30,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<AircraftTestLoader>(
        navalsim::data::DefinitionCategory("aircraft")
    );

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
    );

    navalsim::data::DefinitionDatabase database;

    const auto aircraftResult = registry.loadFiles(
        navalsim::data::DefinitionCategory("aircraft"),
        {aircraftPath},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionBatchResult
    >(aircraftResult));

    CHECK(std::get<navalsim::data::DefinitionBatchResult>(
        aircraftResult
    ).success());

    const auto unitResult = registry.loadFiles(
        navalsim::data::DefinitionCategory("units"),
        {unitsPath},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionBatchResult
    >(unitResult));

    CHECK(std::get<navalsim::data::DefinitionBatchResult>(
        unitResult
    ).success());

    CHECK(database.definitionCount() == 2);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("test-vessel")
    ) != nullptr);

    CHECK(database.find<AircraftTestDefinition>(
        navalsim::data::DefinitionId("test-aircraft")
    ) != nullptr);
}

TEST_CASE("Definition loader registry reports unknown categories") {
    navalsim::data::DefinitionLoaderRegistry registry;
    navalsim::data::DefinitionDatabase database;

    const auto result = registry.loadFiles(
        navalsim::data::DefinitionCategory("unknown"),
        {},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionRegistryError
    >(result));

    const auto& error =
        std::get<navalsim::data::DefinitionRegistryError>(result);

    CHECK(error.category == "unknown");
    CHECK_FALSE(error.message.empty());
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Definition loader registry dispatches collection loaders") {
    TemporaryDirectory directory;

    const auto unitsPath = directory.writeFile(
        "units.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "registry-frigate",
                    "name": "Registry Frigate",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 28,
                    "components": []
                },
                {
                    "id": "registry-aircraft",
                    "name": "Registry Aircraft",
                    "type": "fighter",
                    "environment": "air",
                    "max_speed_knots": 450,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
    );

    navalsim::data::DefinitionDatabase database;

    const auto result = registry.loadFiles(
        navalsim::data::DefinitionCategory("units"),
        {unitsPath},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionBatchResult
    >(result));

    const auto& batch =
        std::get<navalsim::data::DefinitionBatchResult>(result);

    CHECK(batch.success());
    CHECK(batch.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("registry-frigate")
    ) != nullptr);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("registry-aircraft")
    ) != nullptr);
}

TEST_CASE("Definition loader registry dispatches ammunition collection loaders") {
    TemporaryDirectory directory;

    const auto ammunitionPath = directory.writeFile(
        "ammunition.json",
        R"({
            "schema_version": 1,
            "ammunition": [
                {
                    "id": "registry-sidewinder",
                    "name": "Registry Sidewinder",
                    "range": 18,
                    "speed": 850,
                    "is_guided": true,
                    "damage": 80
                },
                {
                    "id": "registry-bomb",
                    "name": "Registry Bomb",
                    "range": 15,
                    "speed": 250,
                    "is_guided": false,
                    "damage": 120
                }
            ]
        })"
    );

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::AmmunitionCollectionLoader>(
        navalsim::data::DefinitionCategory("ammunition")
    );

    navalsim::data::DefinitionDatabase database;

    const auto result = registry.loadFiles(
        navalsim::data::DefinitionCategory("ammunition"),
        {ammunitionPath},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionBatchResult
    >(result));

    const auto& batch =
        std::get<navalsim::data::DefinitionBatchResult>(result);

    CHECK(batch.success());
    CHECK(batch.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    const auto* sidewinder =
        database.find<navalsim::data::AmmunitionDefinition>(
            navalsim::data::DefinitionId("registry-sidewinder")
        );

    REQUIRE(sidewinder != nullptr);
    CHECK(sidewinder->name() == "Registry Sidewinder");
    CHECK(sidewinder->isGuided());

    const auto* bomb =
        database.find<navalsim::data::AmmunitionDefinition>(
            navalsim::data::DefinitionId("registry-bomb")
        );

    REQUIRE(bomb != nullptr);
    CHECK(bomb->name() == "Registry Bomb");
    CHECK_FALSE(bomb->isGuided());
}

TEST_CASE("Definition loader registry dispatches component collection loaders") {
    TemporaryDirectory directory;

    const auto componentsPath = directory.writeFile(
        "components.json",
        R"({
            "schema_version": 1,
            "components": [
                {
                    "id": "registry-engine",
                    "name": "Registry Engine",
                    "category": "damage_only",
                    "type": "engine"
                },
                {
                    "id": "registry-radar",
                    "name": "Registry Radar",
                    "category": "simulation",
                    "type": "radar",
                    "properties": {
                        "range_nm": 120
                    }
                }
            ]
        })"
    );

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::ComponentCollectionLoader>(
        navalsim::data::DefinitionCategory("components")
    );

    navalsim::data::DefinitionDatabase database;

    const auto result = registry.loadFiles(
        navalsim::data::DefinitionCategory("components"),
        {componentsPath},
        database
    );

    REQUIRE(std::holds_alternative<
        navalsim::data::DefinitionBatchResult
    >(result));

    const auto& batch =
        std::get<navalsim::data::DefinitionBatchResult>(result);

    CHECK(batch.success());
    CHECK(batch.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    const auto* engine =
        database.find<navalsim::data::ComponentDefinition>(
            navalsim::data::DefinitionId("registry-engine")
        );

    REQUIRE(engine != nullptr);

    CHECK(
        static_cast<int>(engine->category()) ==
        static_cast<int>(
            navalsim::data::ComponentCategory::DamageOnly
        )
    );

    CHECK(engine->type() == "engine");

    const auto* radar =
        database.find<navalsim::data::ComponentDefinition>(
            navalsim::data::DefinitionId("registry-radar")
        );

    REQUIRE(radar != nullptr);

    CHECK(
        static_cast<int>(radar->category()) ==
        static_cast<int>(
            navalsim::data::ComponentCategory::Simulation
        )
    );

    REQUIRE(radar->properties().has_value());

    REQUIRE(std::holds_alternative<
        navalsim::data::RadarProperties
    >(*radar->properties()));

    CHECK(
        std::get<navalsim::data::RadarProperties>(
            *radar->properties()
        ).rangeNm() == doctest::Approx(120.0)
    );
}

