#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "data/definitions/definition_loading_configuration_loader.hpp"
#include "data/definitions/definition_loading_service.hpp"
#include "data/definitions/vessel_definition.hpp"
#include "data/definitions/vessel_definition_loader.hpp"
#include "data/json/json_reader.hpp"

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        path_ = std::filesystem::temp_directory_path() / "navalsim_loading_configuration_tests";
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::filesystem::remove_all(path_);
    }

    const std::filesystem::path& path() const noexcept {
        return path_;
    }

    std::filesystem::path createDirectory(const std::string& name) const {
        const auto directory = path_ / name;
        std::filesystem::create_directories(directory);
        return directory;
    }

    std::filesystem::path writeFile(
        const std::filesystem::path& directory,
        const std::string& filename,
        const std::string& contents
    ) const {
        const auto path = directory / filename;
        std::ofstream file(path);
        file << contents;
        return path;
    }

private:
    std::filesystem::path path_;
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

navalsim::data::DefinitionLoadingConfiguration loadConfiguration(const std::string& json) {
    auto readResult = navalsim::data::JsonReader::parse(json, "test-config.json");

    if (!std::holds_alternative<navalsim::data::JsonDocument>(readResult)) {
        throw std::runtime_error("Test configuration JSON could not be parsed.");
    }

    auto loadResult = navalsim::data::DefinitionLoadingConfigurationLoader::load(
        std::get<navalsim::data::JsonDocument>(readResult)
    );

    if (!std::holds_alternative<navalsim::data::DefinitionLoadingConfiguration>(loadResult)) {
        throw std::runtime_error("Test configuration failed validation.");
    }

    return std::get<navalsim::data::DefinitionLoadingConfiguration>(std::move(loadResult));
}

}

TEST_CASE("Definition loading configuration parses multiple category sources") {
    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "definitions/vessels"},
            {"category": "aircraft", "directory": "definitions/aircraft"},
            {"category": "vessel", "directory": "extra/vessels"}
        ]
    })");

    REQUIRE(configuration.sources().size() == 3);
    CHECK(configuration.sources()[0].category.value() == "vessel");
    CHECK(configuration.sources()[0].directory == "definitions/vessels");
    CHECK(configuration.sources()[1].category.value() == "aircraft");
    CHECK(configuration.sources()[2].category.value() == "vessel");
}

TEST_CASE("Definition loading configuration rejects invalid source entries") {
    auto readResult = navalsim::data::JsonReader::parse(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "definitions/vessels"},
            {"category": "", "directory": "definitions/aircraft"},
            {"category": "weapon"}
        ]
    })");

    REQUIRE(std::holds_alternative<navalsim::data::JsonDocument>(readResult));

    const auto result = navalsim::data::DefinitionLoadingConfigurationLoader::load(
        std::get<navalsim::data::JsonDocument>(readResult)
    );

    REQUIRE(std::holds_alternative<navalsim::data::JsonValidationResult>(result));
    CHECK_FALSE(std::get<navalsim::data::JsonValidationResult>(result).valid());
}

TEST_CASE("Definition loading configuration rejects unexpected fields") {
    auto readResult = navalsim::data::JsonReader::parse(R"({
        "definition_sources": [],
        "unexpected": true
    })");

    REQUIRE(std::holds_alternative<navalsim::data::JsonDocument>(readResult));

    const auto result = navalsim::data::DefinitionLoadingConfigurationLoader::load(
        std::get<navalsim::data::JsonDocument>(readResult)
    );

    REQUIRE(std::holds_alternative<navalsim::data::JsonValidationResult>(result));
    CHECK_FALSE(std::get<navalsim::data::JsonValidationResult>(result).valid());
}

TEST_CASE("Configured loading discovers and loads JSON definitions") {
    TemporaryDirectory directory;
    const auto vesselDirectory = directory.createDirectory("definitions/vessels");

    directory.writeFile(vesselDirectory, "vessel.json", validVesselJson);
    directory.writeFile(vesselDirectory, "notes.txt", "Not a definition.");

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "definitions/vessels"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;
    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );

    navalsim::data::DefinitionDatabase database;

    const auto report = navalsim::data::loadConfiguredDefinitions(
        configuration,
        directory.path(),
        registry,
        database
    );

    REQUIRE(report.success());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults[0].filesDiscovered == 1);
    CHECK(database.definitionCount() == 1);
    CHECK(database.find<navalsim::data::VesselDefinition>(
        navalsim::data::DefinitionId("test-vessel")
    ) != nullptr);
}

TEST_CASE("Configured loading combines directories for the same category") {
    TemporaryDirectory directory;
    const auto firstDirectory = directory.createDirectory("vessels/first");
    const auto secondDirectory = directory.createDirectory("vessels/second");

    directory.writeFile(firstDirectory, "first.json", validVesselJson);

    directory.writeFile(secondDirectory, "second.json", R"({
        "id": "second-vessel",
        "name": "Second Vessel",
        "dimensions": {"length_m": 120.0, "beam_m": 22.0, "draft_m": 6.0},
        "propulsion": {"maximum_speed_mps": 18.0}
    })");

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "vessels/first"},
            {"category": "vessel", "directory": "vessels/second"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;
    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );

    navalsim::data::DefinitionDatabase database;

    const auto report = navalsim::data::loadConfiguredDefinitions(
        configuration,
        directory.path(),
        registry,
        database
    );

    REQUIRE(report.success());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults[0].filesDiscovered == 2);
    CHECK(database.definitionCount() == 2);
}

TEST_CASE("Configured loading does not partially load a category when a directory is missing") {
    TemporaryDirectory directory;
    const auto existingDirectory = directory.createDirectory("vessels/existing");
    directory.writeFile(existingDirectory, "vessel.json", validVesselJson);

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "vessels/existing"},
            {"category": "vessel", "directory": "vessels/missing"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;
    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );

    navalsim::data::DefinitionDatabase database;

    const auto report = navalsim::data::loadConfiguredDefinitions(
        configuration,
        directory.path(),
        registry,
        database
    );

    CHECK_FALSE(report.success());
    REQUIRE(report.sourceErrors.size() == 1);
    CHECK(report.sourceErrors[0].code == navalsim::data::DefinitionSourceErrorCode::DirectoryNotFound);
    CHECK(report.categoryResults.empty());
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Configured loading reports unregistered categories") {
    TemporaryDirectory directory;

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "unknown", "directory": "definitions/unknown"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;
    navalsim::data::DefinitionDatabase database;

    const auto report = navalsim::data::loadConfiguredDefinitions(
        configuration,
        directory.path(),
        registry,
        database
    );

    CHECK_FALSE(report.success());
    REQUIRE(report.sourceErrors.size() == 1);
    CHECK(report.sourceErrors[0].code == navalsim::data::DefinitionSourceErrorCode::UnknownCategory);
    CHECK(report.sourceErrors[0].category == "unknown");
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Configured loading detects duplicate IDs across directories atomically") {
    TemporaryDirectory directory;
    const auto firstDirectory = directory.createDirectory("vessels/first");
    const auto secondDirectory = directory.createDirectory("vessels/second");

    directory.writeFile(firstDirectory, "first.json", validVesselJson);
    directory.writeFile(secondDirectory, "duplicate.json", validVesselJson);

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "vessel", "directory": "vessels/first"},
            {"category": "vessel", "directory": "vessels/second"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;
    registry.registerLoader<navalsim::data::VesselDefinitionLoader>(
        navalsim::data::DefinitionCategory("vessel")
    );

    navalsim::data::DefinitionDatabase database;

    const auto report = navalsim::data::loadConfiguredDefinitions(
        configuration,
        directory.path(),
        registry,
        database
    );

    CHECK_FALSE(report.success());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(database.definitionCount() == 0);

    const auto* batchResult = std::get_if<navalsim::data::DefinitionBatchResult>(
        &report.categoryResults[0].result
    );

    REQUIRE(batchResult != nullptr);
    CHECK_FALSE(batchResult->success());
    REQUIRE(batchResult->errors.size() == 1);
}

