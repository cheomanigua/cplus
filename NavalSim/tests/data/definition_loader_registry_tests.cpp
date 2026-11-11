#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "data/definitions/definition_loader_registry.hpp"
#include "data/definitions/vessel_definition.hpp"
#include "data/definitions/vessel_definition_loader.hpp"
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

    std::filesystem::path writeFile(const std::string& filename, const std::string& contents) const {
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
    explicit AircraftTestDefinition(navalsim::data::DefinitionId id) : id_(std::move(id)) {}

    const navalsim::data::DefinitionId& id() const noexcept { return id_; }

private:
    navalsim::data::DefinitionId id_;
};

class AircraftTestLoader {
public:
    using DefinitionType = AircraftTestDefinition;

    static std::variant<DefinitionType, navalsim::data::JsonValidationResult> load(
        const navalsim::data::JsonDocument& document
    ) {
        navalsim::data::JsonObjectValidator validator(document.value());
        validator.requireString("id", true);
        validator.allowOnlyFields({"id"});

        auto validation = validator.result();
        if (!validation.valid()) return validation;

        return DefinitionType(navalsim::data::DefinitionId(document.value().at("id").get<std::string>()));
    }
};

const std::string validVesselJson = R"({
    "id": "test-vessel",
    "name": "Test Vessel",
    "dimensions": {
        "length_m": 100.0,
        "beam_m": 20.0,
        "draft_m": 5.0
    },
    "propulsion": {
        "maximum_speed_mps": 15.0
    }
})";

}

TEST_CASE("Definition category rejects empty identifiers") {
    CHECK_THROWS_AS(navalsim::data::DefinitionCategory(""), std::invalid_argument);
}

TEST_CASE("Definition loader registry discovers registered categories") {
    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );
    registry.registerLoader<AircraftTestLoader>(
        navalsim::data::DefinitionCategory("aircraft")
    );

    CHECK(registry.isRegistered(navalsim::data::DefinitionCategory("vessel")));
    CHECK(registry.isRegistered(navalsim::data::DefinitionCategory("aircraft")));
    CHECK_FALSE(registry.isRegistered(navalsim::data::DefinitionCategory("weapon")));

    const auto categories = registry.categories();
    REQUIRE(categories.size() == 2);
    CHECK(categories[0].value() == "aircraft");
    CHECK(categories[1].value() == "vessel");
}

TEST_CASE("Definition loader registry rejects duplicate category registrations") {
    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );

    CHECK_THROWS_AS(
        registry.registerLoader<AircraftTestLoader>(navalsim::data::DefinitionCategory("vessel")),
        std::invalid_argument
    );

    CHECK(registry.categories().size() == 1);
}

TEST_CASE("Definition loader registry dispatches loading by category") {
    TemporaryDirectory directory;
    const auto vesselPath = directory.writeFile("vessel.json", validVesselJson);
    const auto aircraftPath = directory.writeFile("aircraft.json", R"({"id": "test-aircraft"})");

    navalsim::data::DefinitionLoaderRegistry registry;
    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );
    registry.registerLoader<AircraftTestLoader>(
        navalsim::data::DefinitionCategory("aircraft")
    );

    navalsim::data::DefinitionDatabase database;

    const auto vesselResult = registry.loadFiles(
        navalsim::data::DefinitionCategory("vessel"),
        {vesselPath},
        database
    );

    REQUIRE(std::holds_alternative<navalsim::data::DefinitionBatchResult>(vesselResult));
    CHECK(std::get<navalsim::data::DefinitionBatchResult>(vesselResult).success());

    const auto aircraftResult = registry.loadFiles(
        navalsim::data::DefinitionCategory("aircraft"),
        {aircraftPath},
        database
    );

    REQUIRE(std::holds_alternative<navalsim::data::DefinitionBatchResult>(aircraftResult));
    CHECK(std::get<navalsim::data::DefinitionBatchResult>(aircraftResult).success());

    CHECK(database.definitionCount() == 2);
    CHECK(database.find<navalsim::data::VesselDefinition>(
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

    REQUIRE(std::holds_alternative<navalsim::data::DefinitionRegistryError>(result));

    const auto& error = std::get<navalsim::data::DefinitionRegistryError>(result);
    CHECK(error.category == "unknown");
    CHECK_FALSE(error.message.empty());
    CHECK(database.definitionCount() == 0);
}
