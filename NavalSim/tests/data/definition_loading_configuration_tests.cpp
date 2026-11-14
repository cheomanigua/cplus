
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
#include "data/definitions/unit_collection_loader.hpp"
#include "data/definitions/unit_definition.hpp"
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

const std::string validUnitCollectionJson = R"({
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
})";

navalsim::data::DefinitionLoadingConfiguration loadConfiguration(
    const std::string& json
) {
    auto readResult = navalsim::data::JsonReader::parse(
        json,
        "test-config.json"
    );

    if (!std::holds_alternative<navalsim::data::JsonDocument>(readResult)) {
        throw std::runtime_error(
            "Test configuration JSON could not be parsed."
        );
    }

    auto loadResult =
        navalsim::data::DefinitionLoadingConfigurationLoader::load(
            std::get<navalsim::data::JsonDocument>(readResult)
        );

    if (!std::holds_alternative<
        navalsim::data::DefinitionLoadingConfiguration
    >(loadResult)) {
        throw std::runtime_error(
            "Test configuration failed validation."
        );
    }

    return std::get<navalsim::data::DefinitionLoadingConfiguration>(
        std::move(loadResult)
    );
}

}

TEST_CASE("Definition loading configuration parses multiple category sources") {
    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "units", "directory": "definitions/units"},
            {"category": "aircraft", "directory": "definitions/aircraft"},
            {"category": "units", "directory": "extra/units"}
        ]
    })");

    REQUIRE(configuration.sources().size() == 3);
    CHECK(configuration.sources()[0].category.value() == "units");
    CHECK(configuration.sources()[0].directory == "definitions/units");
    CHECK(configuration.sources()[1].category.value() == "aircraft");
    CHECK(configuration.sources()[2].category.value() == "units");
}

TEST_CASE("Definition loading configuration rejects invalid source entries") {
    auto readResult = navalsim::data::JsonReader::parse(R"({
        "definition_sources": [
            {"category": "units", "directory": "definitions/units"},
            {"category": "", "directory": "definitions/aircraft"},
            {"category": "weapon"}
        ]
    })");

    REQUIRE(std::holds_alternative<navalsim::data::JsonDocument>(readResult));

    const auto result =
        navalsim::data::DefinitionLoadingConfigurationLoader::load(
            std::get<navalsim::data::JsonDocument>(readResult)
        );

    REQUIRE(std::holds_alternative<
        navalsim::data::JsonValidationResult
    >(result));

    CHECK_FALSE(
        std::get<navalsim::data::JsonValidationResult>(result).valid()
    );
}

TEST_CASE("Definition loading configuration rejects unexpected fields") {
    auto readResult = navalsim::data::JsonReader::parse(R"({
        "definition_sources": [],
        "unexpected": true
    })");

    REQUIRE(std::holds_alternative<navalsim::data::JsonDocument>(readResult));

    const auto result =
        navalsim::data::DefinitionLoadingConfigurationLoader::load(
            std::get<navalsim::data::JsonDocument>(readResult)
        );

    REQUIRE(std::holds_alternative<
        navalsim::data::JsonValidationResult
    >(result));

    CHECK_FALSE(
        std::get<navalsim::data::JsonValidationResult>(result).valid()
    );
}

TEST_CASE("Configured loading discovers and loads JSON definitions") {
    TemporaryDirectory directory;

    const auto unitsDirectory = directory.createDirectory("definitions/units");

    directory.writeFile(
        unitsDirectory,
        "units.json",
        validUnitCollectionJson
    );

    directory.writeFile(
        unitsDirectory,
        "notes.txt",
        "Not a definition."
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "units", "directory": "definitions/units"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
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

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("test-vessel")
    ) != nullptr);
}

TEST_CASE("Configured loading combines directories for the same category") {
    TemporaryDirectory directory;

    const auto firstDirectory = directory.createDirectory("units/first");
    const auto secondDirectory = directory.createDirectory("units/second");

    directory.writeFile(
        firstDirectory,
        "first.json",
        validUnitCollectionJson
    );

    directory.writeFile(
        secondDirectory,
        "second.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "second-unit",
                    "name": "Second Unit",
                    "type": "destroyer",
                    "environment": "water",
                    "max_speed_knots": 32,
                    "components": []
                }
            ]
        })"
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "units", "directory": "units/first"},
            {"category": "units", "directory": "units/second"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
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

    const auto existingDirectory = directory.createDirectory("units/existing");

    directory.writeFile(
        existingDirectory,
        "units.json",
        validUnitCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "units", "directory": "units/existing"},
            {"category": "units", "directory": "units/missing"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
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

    CHECK(
        report.sourceErrors[0].code ==
        navalsim::data::DefinitionSourceErrorCode::DirectoryNotFound
    );

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

    CHECK(
        report.sourceErrors[0].code ==
        navalsim::data::DefinitionSourceErrorCode::UnknownCategory
    );

    CHECK(report.sourceErrors[0].category == "unknown");
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Configured loading detects duplicate IDs across directories atomically") {
    TemporaryDirectory directory;

    const auto firstDirectory = directory.createDirectory("units/first");
    const auto secondDirectory = directory.createDirectory("units/second");

    directory.writeFile(
        firstDirectory,
        "first.json",
        validUnitCollectionJson
    );

    directory.writeFile(
        secondDirectory,
        "duplicate.json",
        validUnitCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {"category": "units", "directory": "units/first"},
            {"category": "units", "directory": "units/second"}
        ]
    })");

    navalsim::data::DefinitionLoaderRegistry registry;

    registry.registerLoader<navalsim::data::UnitCollectionLoader>(
        navalsim::data::DefinitionCategory("units")
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

    const auto* batchResult =
        std::get_if<navalsim::data::DefinitionBatchResult>(
            &report.categoryResults[0].result
        );

    REQUIRE(batchResult != nullptr);
    CHECK_FALSE(batchResult->success());
    REQUIRE(batchResult->errors.size() == 1);
}

