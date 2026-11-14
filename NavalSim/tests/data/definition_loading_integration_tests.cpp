
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/definition_loader_registry.hpp"
#include "data/definitions/definition_loading_configuration_loader.hpp"
#include "data/definitions/definition_loading_service.hpp"
#include "data/definitions/component_collection_loader.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/unit_collection_loader.hpp"
#include "data/definitions/unit_definition.hpp"
#include "data/json/json_reader.hpp"

namespace {

using namespace navalsim::data;

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        const auto timestamp =
            std::chrono::steady_clock::now().time_since_epoch().count();

        path_ = std::filesystem::temp_directory_path()
            / ("navalsim_definition_loading_" + std::to_string(timestamp));

        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

void writeFile(
    const std::filesystem::path& path,
    const std::string& contents
) {
    std::ofstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Unable to create test file: " + path.string()
        );
    }

    file << contents;

    if (!file) {
        throw std::runtime_error(
            "Unable to write test file: " + path.string()
        );
    }
}

DefinitionLoadingConfiguration loadConfiguration(const std::string& json) {
    auto readResult = JsonReader::parse(json, "<integration-test>");

    const auto* document = std::get_if<JsonDocument>(&readResult);

    if (document == nullptr) {
        throw std::runtime_error(
            "Unable to parse test configuration."
        );
    }

    auto configurationResult =
        DefinitionLoadingConfigurationLoader::load(*document);

    const auto* configuration =
        std::get_if<DefinitionLoadingConfiguration>(&configurationResult);

    if (configuration == nullptr) {
        throw std::runtime_error(
            "Test configuration is invalid."
        );
    }

    return *configuration;
}

constexpr const char* validUnitCollectionJson = R"({
    "schema_version": 1,
    "units": [
        {
            "id": "test_frigate",
            "name": "Test Frigate",
            "type": "frigate",
            "environment": "water",
            "max_speed_knots": 30,
            "fuel": null,
            "components": []
        },
        {
            "id": "test_submarine",
            "name": "Test Submarine",
            "type": "submarine",
            "environment": "water",
            "max_speed_knots": 25,
            "fuel": null,
            "components": []
        }
    ]
})";

constexpr const char* invalidUnitCollectionJson = R"({
    "schema_version": 1,
    "units": [
        {
            "id": "invalid_unit",
            "name": "Invalid Unit",
            "type": "frigate",
            "environment": "water",
            "components": []
        }
    ]
})";

constexpr const char* validComponentCollectionJson = R"({
    "schema_version": 1,
    "components": [
        {
            "id": "test_engine",
            "name": "Test Engine",
            "category": "damage_only",
            "type": "engine"
        },
        {
            "id": "test_radar",
            "name": "Test Radar",
            "category": "simulation",
            "type": "radar",
            "properties": {
                "range_nm": 120
            }
        }
    ]
})";

} // namespace

TEST_CASE("End-to-end loading discovers, loads, and retrieves unit definitions") {
    TemporaryDirectory temporaryDirectory;

    const auto unitDirectory = temporaryDirectory.path() / "units";
    std::filesystem::create_directories(unitDirectory);

    writeFile(
        unitDirectory / "naval_units.json",
        validUnitCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "units",
                "directory": "units"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;

    registry.registerLoader<UnitCollectionLoader>(
        DefinitionCategory("units")
    );

    DefinitionDatabase database;

    const auto report = loadConfiguredDefinitions(
        configuration,
        temporaryDirectory.path(),
        registry,
        database
    );

    REQUIRE(report.success());
    REQUIRE(report.sourceErrors.empty());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 1);

    const auto* batchResult =
        std::get_if<DefinitionBatchResult>(
            &report.categoryResults.front().result
        );

    REQUIRE(batchResult != nullptr);
    CHECK(batchResult->success());
    CHECK(batchResult->definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    const auto* frigate =
        database.find<UnitDefinition>(DefinitionId("test_frigate"));

    REQUIRE(frigate != nullptr);
    CHECK(frigate->name() == "Test Frigate");
    CHECK(frigate->type() == "frigate");
	CHECK(static_cast<int>(frigate->environment()) == static_cast<int>(UnitEnvironment::Water));
    CHECK(frigate->maxSpeedKnots() == doctest::Approx(30.0));
    CHECK_FALSE(frigate->fuel().has_value());

    const auto* submarine =
        database.find<UnitDefinition>(DefinitionId("test_submarine"));

    REQUIRE(submarine != nullptr);
    CHECK(submarine->name() == "Test Submarine");
    CHECK(submarine->type() == "submarine");
    CHECK(submarine->maxSpeedKnots() == doctest::Approx(25.0));
}

TEST_CASE("End-to-end loading does not partially load a category containing an invalid definition") {
    TemporaryDirectory temporaryDirectory;

    const auto unitDirectory = temporaryDirectory.path() / "units";
    std::filesystem::create_directories(unitDirectory);

    writeFile(
        unitDirectory / "01_valid.json",
        validUnitCollectionJson
    );

    writeFile(
        unitDirectory / "02_invalid.json",
        invalidUnitCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "units",
                "directory": "units"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;

    registry.registerLoader<UnitCollectionLoader>(
        DefinitionCategory("units")
    );

    DefinitionDatabase database;

    const auto report = loadConfiguredDefinitions(
        configuration,
        temporaryDirectory.path(),
        registry,
        database
    );

    CHECK_FALSE(report.success());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 2);

    const auto* batchResult =
        std::get_if<DefinitionBatchResult>(
            &report.categoryResults.front().result
        );

    REQUIRE(batchResult != nullptr);
    CHECK_FALSE(batchResult->success());
    CHECK_FALSE(batchResult->errors.empty());

    CHECK(database.definitionCount() == 0);

    CHECK(
        database.find<UnitDefinition>(
            DefinitionId("test_frigate")
        ) == nullptr
    );

    CHECK(
        database.find<UnitDefinition>(
            DefinitionId("test_submarine")
        ) == nullptr
    );
}

TEST_CASE("End-to-end loading discovers and loads unit collections through the registry") {
    TemporaryDirectory temporaryDirectory;

    const auto unitDirectory = temporaryDirectory.path() / "units";
    std::filesystem::create_directories(unitDirectory);

    writeFile(
        unitDirectory / "naval_units.json",
        validUnitCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "units",
                "directory": "units"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;

    registry.registerLoader<UnitCollectionLoader>(
        DefinitionCategory("units")
    );

    DefinitionDatabase database;

    const auto report = loadConfiguredDefinitions(
        configuration,
        temporaryDirectory.path(),
        registry,
        database
    );

    REQUIRE(report.success());
    REQUIRE(report.sourceErrors.empty());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 1);

    const auto* batchResult =
        std::get_if<DefinitionBatchResult>(
            &report.categoryResults.front().result
        );

    REQUIRE(batchResult != nullptr);
    CHECK(batchResult->success());
    CHECK(batchResult->definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    CHECK(
        database.find<UnitDefinition>(
            DefinitionId("test_frigate")
        ) != nullptr
    );

    CHECK(
        database.find<UnitDefinition>(
            DefinitionId("test_submarine")
        ) != nullptr
    );
}

TEST_CASE("End-to-end loading discovers and loads component collections through the registry") {
    TemporaryDirectory temporaryDirectory;

    const auto componentDirectory =
        temporaryDirectory.path() / "components";

    std::filesystem::create_directories(componentDirectory);

    writeFile(
        componentDirectory / "naval_components.json",
        validComponentCollectionJson
    );

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "components",
                "directory": "components"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;

    registry.registerLoader<ComponentCollectionLoader>(
        DefinitionCategory("components")
    );

    DefinitionDatabase database;

    const auto report = loadConfiguredDefinitions(
        configuration,
        temporaryDirectory.path(),
        registry,
        database
    );

    REQUIRE(report.success());
    REQUIRE(report.sourceErrors.empty());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 1);

    const auto* batchResult =
        std::get_if<DefinitionBatchResult>(
            &report.categoryResults.front().result
        );

    REQUIRE(batchResult != nullptr);
    CHECK(batchResult->success());
    CHECK(batchResult->definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    const auto* engine =
        database.find<ComponentDefinition>(
            DefinitionId("test_engine")
        );

    REQUIRE(engine != nullptr);
    CHECK(engine->type() == "engine");

    const auto* radar =
        database.find<ComponentDefinition>(
            DefinitionId("test_radar")
        );

    REQUIRE(radar != nullptr);
    REQUIRE(radar->properties().has_value());

    CHECK(
        std::get<RadarProperties>(*radar->properties()).rangeNm()
        == doctest::Approx(120.0)
    );
}

