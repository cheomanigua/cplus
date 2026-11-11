#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/definitions/definition_batch_loader.hpp"
#include "data/definitions/vessel_definition.hpp"
#include "data/definitions/vessel_definition_loader.hpp"

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        path_ = std::filesystem::temp_directory_path() / "navalsim_definition_batch_tests";
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

TEST_CASE("Generic batch loader loads multiple definitions") {
    TemporaryDirectory directory;
    const auto first = directory.writeFile("first.json", validVesselJson);
    const auto second = directory.writeFile(
        "second.json",
        R"({
            "id": "second-vessel",
            "name": "Second Vessel",
            "dimensions": {"length_m": 120.0, "beam_m": 22.0, "draft_m": 6.0},
            "propulsion": {"maximum_speed_mps": 18.0}
        })"
    );

    navalsim::data::DefinitionDatabase database;
    const auto result = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {first, second},
        database
    );

    CHECK(result.success());
    CHECK(result.errors.empty());
    CHECK(result.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);
    CHECK(database.find<navalsim::data::VesselDefinition>(navalsim::data::DefinitionId("test-vessel")) != nullptr);
    CHECK(database.find<navalsim::data::VesselDefinition>(navalsim::data::DefinitionId("second-vessel")) != nullptr);
}

TEST_CASE("Generic batch loader collects errors and does not partially populate database") {
    TemporaryDirectory directory;
    const auto valid = directory.writeFile("valid.json", validVesselJson);
    const auto invalidJson = directory.writeFile("invalid-json.json", "{ invalid json");
    const auto invalidDefinition = directory.writeFile("invalid-definition.json", R"({"id": "missing-fields"})");

    navalsim::data::DefinitionDatabase database;
    const auto result = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {valid, invalidJson, invalidDefinition},
        database
    );

    CHECK_FALSE(result.success());
    CHECK(result.errors.size() == 2);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);
    CHECK(result.errors[0].source == invalidJson.string());
    CHECK(result.errors[1].source == invalidDefinition.string());
}

TEST_CASE("Generic batch loader detects duplicate IDs within a batch") {
    TemporaryDirectory directory;
    const auto first = directory.writeFile("first.json", validVesselJson);
    const auto second = directory.writeFile("second.json", validVesselJson);

    navalsim::data::DefinitionDatabase database;
    const auto result = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {first, second},
        database
    );

    REQUIRE_FALSE(result.success());
    REQUIRE(result.errors.size() == 1);
    CHECK(std::holds_alternative<navalsim::data::DuplicateDefinitionIdError>(result.errors[0].detail));
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);

    const auto& duplicate = std::get<navalsim::data::DuplicateDefinitionIdError>(result.errors[0].detail);
    CHECK(duplicate.id == "test-vessel");
    CHECK(duplicate.conflictingSource == first.string());
}

TEST_CASE("Generic batch loader detects IDs already present in the database") {
    TemporaryDirectory directory;
    const auto initial = directory.writeFile("initial.json", validVesselJson);
    const auto duplicate = directory.writeFile("duplicate.json", validVesselJson);
    const auto additional = directory.writeFile(
        "additional.json",
        R"({
            "id": "additional-vessel",
            "name": "Additional Vessel",
            "dimensions": {"length_m": 90.0, "beam_m": 18.0, "draft_m": 4.0},
            "propulsion": {"maximum_speed_mps": 12.0}
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto initialResult = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {initial},
        database
    );
    REQUIRE(initialResult.success());
    REQUIRE(database.definitionCount() == 1);

    const auto result = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {duplicate, additional},
        database
    );

    CHECK_FALSE(result.success());
    CHECK(result.errors.size() == 1);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 1);
    CHECK(database.find<navalsim::data::VesselDefinition>(navalsim::data::DefinitionId("additional-vessel")) == nullptr);

    const auto& duplicateError = std::get<navalsim::data::DuplicateDefinitionIdError>(result.errors[0].detail);
    CHECK(duplicateError.id == "test-vessel");
    CHECK(duplicateError.conflictingSource == "<existing database>");
}

TEST_CASE("Generic batch loader accepts an empty file collection") {
    navalsim::data::DefinitionDatabase database;

    const auto result = navalsim::data::loadDefinitionFiles<navalsim::data::VesselDefinitionLoader>(
        {},
        database
    );

    CHECK(result.success());
    CHECK(result.definitionsLoaded == 0);
    CHECK(result.errors.empty());
    CHECK(database.definitionCount() == 0);
}
