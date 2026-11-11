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
#include "data/definitions/vessel_definition.hpp"
#include "data/definitions/vessel_definition_loader.hpp"
#include "data/json/json_reader.hpp"

namespace {

using namespace navalsim::data;

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() / ("navalsim_definition_loading_" + std::to_string(timestamp));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path);
    if (!file.is_open()) throw std::runtime_error("Unable to create test file: " + path.string());
    file << contents;
    if (!file) throw std::runtime_error("Unable to write test file: " + path.string());
}

DefinitionLoadingConfiguration loadConfiguration(const std::string& json) {
    auto readResult = JsonReader::parse(json, "<integration-test>");
    const auto* document = std::get_if<JsonDocument>(&readResult);
    if (document == nullptr) throw std::runtime_error("Unable to parse test configuration.");

    auto configurationResult = DefinitionLoadingConfigurationLoader::load(*document);
    const auto* configuration = std::get_if<DefinitionLoadingConfiguration>(&configurationResult);
    if (configuration == nullptr) throw std::runtime_error("Test configuration is invalid.");

    return *configuration;
}

constexpr const char* validVesselJson = R"({
    "id": "test_destroyer",
    "name": "Test Destroyer",
    "dimensions": {
        "length_m": 150.0,
        "beam_m": 20.0,
        "draft_m": 7.0
    },
    "propulsion": {
        "maximum_speed_mps": 16.0
    }
})";

constexpr const char* invalidVesselJson = R"({
    "id": "invalid_vessel",
    "name": "Invalid Vessel",
    "dimensions": {
        "length_m": 150.0,
        "beam_m": 20.0,
        "draft_m": 7.0
    }
})";

} // namespace

TEST_CASE("End-to-end loading discovers, loads, and retrieves vessel definitions") {
    TemporaryDirectory temporaryDirectory;
    const auto vesselDirectory = temporaryDirectory.path() / "vessels";
    std::filesystem::create_directories(vesselDirectory);

    writeFile(vesselDirectory / "destroyer.json", validVesselJson);

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "vessels",
                "directory": "vessels"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;
    registry.registerLoader<VesselDefinitionLoader>(DefinitionCategory("vessels"));

    DefinitionDatabase database;
    const auto report = loadConfiguredDefinitions(configuration, temporaryDirectory.path(), registry, database);

    REQUIRE(report.success());
    REQUIRE(report.sourceErrors.empty());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 1);

    const auto* batchResult = std::get_if<DefinitionBatchResult>(&report.categoryResults.front().result);
    REQUIRE(batchResult != nullptr);
    CHECK(batchResult->success());
    CHECK(batchResult->definitionsLoaded == 1);
    CHECK(database.definitionCount() == 1);

    const auto* vessel = database.find<VesselDefinition>(DefinitionId("test_destroyer"));
    REQUIRE(vessel != nullptr);
    CHECK(vessel->name() == "Test Destroyer");
    CHECK(vessel->dimensions().length().meters() == doctest::Approx(150.0));
    CHECK(vessel->dimensions().beam().meters() == doctest::Approx(20.0));
    CHECK(vessel->dimensions().draft().meters() == doctest::Approx(7.0));
    CHECK(vessel->propulsion().maximumSpeedMetersPerSecond() == doctest::Approx(16.0));
}

TEST_CASE("End-to-end loading does not partially load a category containing an invalid definition") {
    TemporaryDirectory temporaryDirectory;
    const auto vesselDirectory = temporaryDirectory.path() / "vessels";
    std::filesystem::create_directories(vesselDirectory);

    writeFile(vesselDirectory / "01_valid.json", validVesselJson);
    writeFile(vesselDirectory / "02_invalid.json", invalidVesselJson);

    const auto configuration = loadConfiguration(R"({
        "definition_sources": [
            {
                "category": "vessels",
                "directory": "vessels"
            }
        ]
    })");

    DefinitionLoaderRegistry registry;
    registry.registerLoader<VesselDefinitionLoader>(DefinitionCategory("vessels"));

    DefinitionDatabase database;
    const auto report = loadConfiguredDefinitions(configuration, temporaryDirectory.path(), registry, database);

    CHECK_FALSE(report.success());
    REQUIRE(report.categoryResults.size() == 1);
    CHECK(report.categoryResults.front().filesDiscovered == 2);

    const auto* batchResult = std::get_if<DefinitionBatchResult>(&report.categoryResults.front().result);
    REQUIRE(batchResult != nullptr);
    CHECK_FALSE(batchResult->success());
    CHECK_FALSE(batchResult->errors.empty());

    CHECK(database.definitionCount() == 0);
    CHECK(database.find<VesselDefinition>(DefinitionId("test_destroyer")) == nullptr);
}
