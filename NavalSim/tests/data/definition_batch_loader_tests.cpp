
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/definitions/definition_batch_loader.hpp"
#include "data/definitions/unit_collection_loader.hpp"
#include "data/definitions/unit_definition.hpp"

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

}

TEST_CASE("Generic batch loader loads multiple definitions") {
    TemporaryDirectory directory;

    const auto first = directory.writeFile(
        "first.json",
        validUnitCollectionJson
    );

    const auto second = directory.writeFile(
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

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({first, second}, database);

    CHECK(result.success());
    CHECK(result.errors.empty());
    CHECK(result.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("test-vessel")
    ) != nullptr);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("second-unit")
    ) != nullptr);
}

TEST_CASE("Generic batch loader collects errors and does not partially populate database") {
    TemporaryDirectory directory;

    const auto valid = directory.writeFile(
        "valid.json",
        validUnitCollectionJson
    );

    const auto invalidJson = directory.writeFile(
        "invalid-json.json",
        "{ invalid json"
    );

    const auto invalidDefinition = directory.writeFile(
        "invalid-definition.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "missing-fields"
                }
            ]
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({valid, invalidJson, invalidDefinition}, database);

    CHECK_FALSE(result.success());
    CHECK(result.errors.size() == 2);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);
    CHECK(result.errors[0].source == invalidJson.string());
    CHECK(result.errors[1].source == invalidDefinition.string());
}

TEST_CASE("Generic batch loader detects duplicate IDs within a batch") {
    TemporaryDirectory directory;

    const auto first = directory.writeFile(
        "first.json",
        validUnitCollectionJson
    );

    const auto second = directory.writeFile(
        "second.json",
        validUnitCollectionJson
    );

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({first, second}, database);

    REQUIRE_FALSE(result.success());
    REQUIRE(result.errors.size() == 1);

    CHECK(std::holds_alternative<
        navalsim::data::DuplicateDefinitionIdError
    >(result.errors[0].detail));

    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);

    const auto& duplicate =
        std::get<navalsim::data::DuplicateDefinitionIdError>(
            result.errors[0].detail
        );

    CHECK(duplicate.id == "test-vessel");
    CHECK(duplicate.conflictingSource == first.string());
}

TEST_CASE("Generic batch loader detects IDs already present in the database") {
    TemporaryDirectory directory;

    const auto initial = directory.writeFile(
        "initial.json",
        validUnitCollectionJson
    );

    const auto duplicate = directory.writeFile(
        "duplicate.json",
        validUnitCollectionJson
    );

    const auto additional = directory.writeFile(
        "additional.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "additional-unit",
                    "name": "Additional Unit",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 25,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto initialResult =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({initial}, database);

    REQUIRE(initialResult.success());
    REQUIRE(database.definitionCount() == 1);

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({duplicate, additional}, database);

    CHECK_FALSE(result.success());
    CHECK(result.errors.size() == 1);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 1);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("additional-unit")
    ) == nullptr);

    const auto& duplicateError =
        std::get<navalsim::data::DuplicateDefinitionIdError>(
            result.errors[0].detail
        );

    CHECK(duplicateError.id == "test-vessel");
    CHECK(duplicateError.conflictingSource == "<existing database>");
}

TEST_CASE("Generic batch loader accepts an empty file collection") {
    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({}, database);

    CHECK(result.success());
    CHECK(result.definitionsLoaded == 0);
    CHECK(result.errors.empty());
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Generic batch loader loads multiple definitions from a collection file") {
    TemporaryDirectory directory;

    const auto collection = directory.writeFile(
        "units.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "test-frigate",
                    "name": "Test Frigate",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 30,
                    "components": []
                },
                {
                    "id": "test-aircraft",
                    "name": "Test Aircraft",
                    "type": "fighter",
                    "environment": "air",
                    "max_speed_knots": 500,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({collection}, database);

    CHECK(result.success());
    CHECK(result.definitionsLoaded == 2);
    CHECK(database.definitionCount() == 2);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("test-frigate")
    ) != nullptr);

    CHECK(database.find<navalsim::data::UnitDefinition>(
        navalsim::data::DefinitionId("test-aircraft")
    ) != nullptr);
}

TEST_CASE("Collection batch loader rejects duplicate IDs across files atomically") {
    TemporaryDirectory directory;

    const auto first = directory.writeFile(
        "first.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "shared-unit",
                    "name": "First Unit",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 20,
                    "components": []
                }
            ]
        })"
    );

    const auto second = directory.writeFile(
        "second.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "shared-unit",
                    "name": "Second Unit",
                    "type": "destroyer",
                    "environment": "water",
                    "max_speed_knots": 30,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({first, second}, database);

    REQUIRE_FALSE(result.success());
    REQUIRE(result.errors.size() == 1);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);

    REQUIRE(std::holds_alternative<
        navalsim::data::DuplicateDefinitionIdError
    >(result.errors[0].detail));

    const auto& duplicate =
        std::get<navalsim::data::DuplicateDefinitionIdError>(
            result.errors[0].detail
        );

    CHECK(duplicate.id == "shared-unit");
    CHECK(duplicate.conflictingSource == first.string());
}

TEST_CASE("Collection batch loader preserves atomicity for invalid collection files") {
    TemporaryDirectory directory;

    const auto valid = directory.writeFile(
        "valid.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "valid-unit",
                    "name": "Valid Unit",
                    "type": "frigate",
                    "environment": "water",
                    "max_speed_knots": 20,
                    "components": []
                }
            ]
        })"
    );

    const auto invalid = directory.writeFile(
        "invalid.json",
        R"({
            "schema_version": 1,
            "units": [
                {
                    "id": "invalid-unit",
                    "name": "Invalid Unit",
                    "type": "frigate",
                    "environment": "space",
                    "max_speed_knots": 20,
                    "components": []
                }
            ]
        })"
    );

    navalsim::data::DefinitionDatabase database;

    const auto result =
        navalsim::data::loadDefinitionFiles<
            navalsim::data::UnitCollectionLoader
        >({valid, invalid}, database);

    CHECK_FALSE(result.success());
    CHECK(result.errors.size() == 1);
    CHECK(result.definitionsLoaded == 0);
    CHECK(database.definitionCount() == 0);
    CHECK(result.errors[0].source == invalid.string());
}

