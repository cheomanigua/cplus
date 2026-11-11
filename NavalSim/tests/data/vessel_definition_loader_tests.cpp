#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <string>
#include <utility>
#include <variant>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/definition_loader.hpp"
#include "data/definitions/vessel_definition_loader.hpp"
#include "data/json/json_reader.hpp"

using namespace navalsim::data;

namespace {

constexpr const char* validVesselJson = R"({
    "id": "destroyer.ddg51",
    "name": "Arleigh Burke-class Destroyer",
    "dimensions": {
        "length_m": 155.0,
        "beam_m": 20.0,
        "draft_m": 9.5
    },
    "propulsion": {
        "maximum_speed_mps": 30.0
    }
})";

JsonDocument parseDocument(const std::string& json) {
    auto result = JsonReader::parse(json);
    REQUIRE(std::holds_alternative<JsonDocument>(result));
    return std::get<JsonDocument>(std::move(result));
}

const JsonValidationResult& validationErrors(const VesselDefinitionLoadResult& result) {
    REQUIRE(std::holds_alternative<JsonValidationResult>(result));
    return std::get<JsonValidationResult>(result);
}

}

TEST_CASE("Vessel definition loader converts valid JSON") {
    const JsonDocument document = parseDocument(validVesselJson);
    const auto result = VesselDefinitionLoader::load(document);

    REQUIRE(std::holds_alternative<VesselDefinition>(result));

    const auto& definition = std::get<VesselDefinition>(result);
    CHECK(definition.id().value() == "destroyer.ddg51");
    CHECK(definition.name() == "Arleigh Burke-class Destroyer");
    CHECK(definition.dimensions().length().meters() == doctest::Approx(155.0));
    CHECK(definition.dimensions().beam().meters() == doctest::Approx(20.0));
    CHECK(definition.dimensions().draft().meters() == doctest::Approx(9.5));
    CHECK(definition.propulsion().maximumSpeedMetersPerSecond() == doctest::Approx(30.0));
}

TEST_CASE("Vessel definition loader rejects missing required fields") {
    const JsonDocument document = parseDocument(R"({
        "name": "Destroyer",
        "dimensions": {
            "length_m": 155.0,
            "beam_m": 20.0,
            "draft_m": 9.5
        },
        "propulsion": {
            "maximum_speed_mps": 30.0
        }
    })");

    const auto result = VesselDefinitionLoader::load(document);
    const auto& errors = validationErrors(result);

    CHECK_FALSE(errors.valid());
    REQUIRE_FALSE(errors.issues().empty());
    CHECK(errors.issues()[0].path == "$.id");
}

TEST_CASE("Vessel definition loader rejects a non-object root") {
    const JsonDocument document = parseDocument(R"(["not", "an", "object"])");
    const auto result = VesselDefinitionLoader::load(document);

    CHECK_FALSE(validationErrors(result).valid());
}

TEST_CASE("Vessel definition loader rejects incorrect field types") {
    const JsonDocument document = parseDocument(R"({
        "id": 123,
        "name": "Destroyer",
        "dimensions": {
            "length_m": "155",
            "beam_m": 20.0,
            "draft_m": 9.5
        },
        "propulsion": {
            "maximum_speed_mps": 30.0
        }
    })");

    const auto result = VesselDefinitionLoader::load(document);
    const auto& errors = validationErrors(result);

    CHECK_FALSE(errors.valid());
    CHECK(errors.issues().size() >= 2);
}

TEST_CASE("Vessel definition loader rejects non-positive measurements") {
    const JsonDocument document = parseDocument(R"({
        "id": "destroyer.ddg51",
        "name": "Destroyer",
        "dimensions": {
            "length_m": 155.0,
            "beam_m": 0.0,
            "draft_m": 9.5
        },
        "propulsion": {
            "maximum_speed_mps": 30.0
        }
    })");

    const auto result = VesselDefinitionLoader::load(document);
    const auto& errors = validationErrors(result);

    CHECK_FALSE(errors.valid());
    REQUIRE_FALSE(errors.issues().empty());
    CHECK(errors.issues()[0].path == "$.dimensions.beam_m");
}

TEST_CASE("Vessel definition loader rejects unknown fields") {
    const JsonDocument document = parseDocument(R"({
        "id": "destroyer.ddg51",
        "name": "Destroyer",
        "unexpected": true,
        "dimensions": {
            "length_m": 155.0,
            "beam_m": 20.0,
            "draft_m": 9.5
        },
        "propulsion": {
            "maximum_speed_mps": 30.0,
            "extra": 1
        }
    })");

    const auto result = VesselDefinitionLoader::load(document);
    const auto& errors = validationErrors(result);

    CHECK_FALSE(errors.valid());
    CHECK(errors.issues().size() >= 2);
}

TEST_CASE("Vessel definition loader rejects incorrectly typed nested objects") {
    const JsonDocument document = parseDocument(R"({
        "id": "destroyer.ddg51",
        "name": "Destroyer",
        "dimensions": [],
        "propulsion": {
            "maximum_speed_mps": 30.0
        }
    })");

    const auto result = VesselDefinitionLoader::load(document);
    const auto& errors = validationErrors(result);

    CHECK_FALSE(errors.valid());
    REQUIRE_FALSE(errors.issues().empty());
    CHECK(errors.issues()[0].path == "$.dimensions");
}

TEST_CASE("Loaded vessel definitions can be added to the generic database") {
    const JsonDocument document = parseDocument(validVesselJson);
    const auto result = VesselDefinitionLoader::load(document);

    REQUIRE(std::holds_alternative<VesselDefinition>(result));

    DefinitionDatabase database;
    database.add(std::get<VesselDefinition>(result));

    const VesselDefinition* stored =
        database.find<VesselDefinition>(DefinitionId("destroyer.ddg51"));

    REQUIRE(stored != nullptr);
    CHECK(stored->name() == "Arleigh Burke-class Destroyer");
    CHECK(database.definitionCount() == 1);
}

TEST_CASE("Vessel definition loader satisfies the generic loader interface") {
    static_assert(DefinitionLoader<VesselDefinitionLoader>);
    CHECK(true);
}

TEST_CASE("Generic JSON loading parses and converts a vessel definition") {
    const auto result = loadDefinitionJson<VesselDefinitionLoader>(validVesselJson);

    REQUIRE(std::holds_alternative<VesselDefinition>(result));

    const auto& definition = std::get<VesselDefinition>(result);
    CHECK(definition.id().value() == "destroyer.ddg51");
    CHECK(definition.name() == "Arleigh Burke-class Destroyer");
}

TEST_CASE("Generic JSON loading reports JSON parsing errors") {
    const auto result = loadDefinitionJson<VesselDefinitionLoader>(
        R"({"id": "destroyer.ddg51", invalid})",
        "invalid-vessel.json"
    );

    REQUIRE(std::holds_alternative<DataError>(result));

    const auto& error = std::get<DataError>(result);
    CHECK(error.code == DataErrorCode::JsonParseFailed);
    CHECK(error.source == "invalid-vessel.json");
}

TEST_CASE("Generic JSON loading reports definition validation errors") {
    const auto result = loadDefinitionJson<VesselDefinitionLoader>(
        R"({
            "id": "destroyer.ddg51",
            "name": "Destroyer",
            "dimensions": {
                "length_m": 155.0,
                "beam_m": 0.0,
                "draft_m": 9.5
            },
            "propulsion": {
                "maximum_speed_mps": 30.0
            }
        })"
    );

    REQUIRE(std::holds_alternative<JsonValidationResult>(result));

    const auto& validation = std::get<JsonValidationResult>(result);
    CHECK_FALSE(validation.valid());
    REQUIRE_FALSE(validation.issues().empty());
}

TEST_CASE("Generic file loading reports file access errors") {
    const auto path = std::filesystem::temp_directory_path() /
        "navalsim_missing_vessel_definition_4_7.json";

    const auto result = loadDefinitionFile<VesselDefinitionLoader>(path);

    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK(std::get<DataError>(result).code == DataErrorCode::FileOpenFailed);
}
