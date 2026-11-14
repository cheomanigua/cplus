#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <variant>

#include <nlohmann/json.hpp>
#include <vector>

#include "data/definitions/unit_collection_loader.hpp"

using namespace navalsim::data;

namespace {

constexpr const char* validCollection = R"({
    "schema_version": 1,
    "units": [
        {
            "id": "hms_intrepid",
            "name": "HMS Intrepid",
            "type": "frigate",
            "environment": "water",
            "max_speed_knots": 30,
            "fuel": {
                "capacity": 120000,
                "reference_speed_knots": 20,
                "consumption_per_hour": 1500
            },
            "components": [
                {
                    "component_id": "marine_engine",
                    "quantity": 2
                }
            ]
        },
        {
            "id": "f16_example",
            "name": "Example Fighter",
            "type": "fighter",
            "environment": "air",
            "max_speed_knots": 1200,
            "fuel": {
                "capacity": 3200,
                "reference_speed_knots": 600,
                "consumption_per_hour": 900
            },
            "components": []
        }
    ]
})";

bool hasValidationErrors(const UnitCollectionLoadResult& result) {
    return std::holds_alternative<JsonValidationResult>(result)
        && !std::get<JsonValidationResult>(result).valid();
}

std::string singleUnitJson(
    const std::string& environment = "water",
    const std::string& extraFields = ""
) {
    return R"({"schema_version":1,"units":[{"id":"u","name":"Unit","type":"test","environment":")"
        + environment
        + R"(","max_speed_knots":10,"fuel":{"capacity":100,"reference_speed_knots":5,"consumption_per_hour":2},"components":[])"
        + extraFields
        + R"(}]})";
}

} // namespace

TEST_CASE("Unit collection loader converts all units in a collection") {
    const auto result = loadUnitCollectionJson(validCollection);

    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(result));
    const auto& units = std::get<std::vector<UnitDefinition>>(result);

    REQUIRE(units.size() == 2);

    CHECK(units[0].id().value() == "hms_intrepid");
    CHECK(units[0].name() == "HMS Intrepid");
    CHECK(units[0].type() == "frigate");
    CHECK((units[0].environment() == UnitEnvironment::Water));
    CHECK(units[0].maxSpeedKnots() == doctest::Approx(30.0));

    REQUIRE(units[0].fuel().has_value());
    CHECK(units[0].fuel()->capacity == doctest::Approx(120000.0));
    CHECK(units[0].fuel()->referenceSpeedKnots == doctest::Approx(20.0));
    CHECK(units[0].fuel()->consumptionPerHour == doctest::Approx(1500.0));

    REQUIRE(units[0].components().size() == 1);
    CHECK(units[0].components()[0].componentId().value() == "marine_engine");
    CHECK(units[0].components()[0].quantity() == 2);
    CHECK((units[1].environment() == UnitEnvironment::Air));
    REQUIRE(units[1].fuel().has_value());
    CHECK(units[1].fuel()->capacity == doctest::Approx(3200.0));
}

TEST_CASE("Unit collection loader accepts all supported environments") {
    for (const std::string environment : {"water", "air", "land"}) {
        const auto result = loadUnitCollectionJson(singleUnitJson(environment));
        REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(result));
        REQUIRE(std::get<std::vector<UnitDefinition>>(result).size() == 1);
    }
}

TEST_CASE("Unit collection loader accepts omitted fuel and explicit null fuel") {
    auto omittedFuel = nlohmann::json::parse(singleUnitJson());
    omittedFuel["units"][0].erase("fuel");

    const auto omittedResult = loadUnitCollectionJson(omittedFuel.dump());
    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(omittedResult));
    REQUIRE(std::get<std::vector<UnitDefinition>>(omittedResult).size() == 1);
    CHECK_FALSE(std::get<std::vector<UnitDefinition>>(omittedResult)[0].fuel().has_value());

    auto nullFuel = nlohmann::json::parse(singleUnitJson());
    nullFuel["units"][0]["fuel"] = nullptr;

    const auto nullResult = loadUnitCollectionJson(nullFuel.dump());
    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(nullResult));
    REQUIRE(std::get<std::vector<UnitDefinition>>(nullResult).size() == 1);
    CHECK_FALSE(std::get<std::vector<UnitDefinition>>(nullResult)[0].fuel().has_value());
}

TEST_CASE("Unit collection loader accepts zero maximum speed") {
    auto json = nlohmann::json::parse(singleUnitJson());
    json["units"][0]["max_speed_knots"] = 0;

    const auto result = loadUnitCollectionJson(json.dump());
    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(result));
    REQUIRE(std::get<std::vector<UnitDefinition>>(result).size() == 1);
    CHECK(std::get<std::vector<UnitDefinition>>(result)[0].maxSpeedKnots() == doctest::Approx(0.0));
}

TEST_CASE("Unit collection loader rejects malformed JSON") {
    const auto result = loadUnitCollectionJson("{ invalid");
    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK((std::get<DataError>(result).code == DataErrorCode::JsonParseFailed));
}

TEST_CASE("Unit collection loader rejects non-object roots and missing collection fields") {
    CHECK(hasValidationErrors(loadUnitCollectionJson("[]")));
    CHECK(hasValidationErrors(loadUnitCollectionJson(R"({"units":[]})")));
}

TEST_CASE("Unit collection loader rejects unsupported schema versions and unknown root fields") {
    CHECK(hasValidationErrors(loadUnitCollectionJson(R"({"schema_version":2,"units":[]})")));
    CHECK(hasValidationErrors(loadUnitCollectionJson(R"({"schema_version":1,"units":[],"extra":true})")));
}

TEST_CASE("Unit collection loader rejects missing unit fields and wrong types") {
    CHECK(hasValidationErrors(loadUnitCollectionJson(R"({"schema_version":1,"units":[{"id":"u"}]})")));
    CHECK(hasValidationErrors(loadUnitCollectionJson(R"({"schema_version":1,"units":["not an object"]})")));
}

TEST_CASE("Unit collection loader rejects the superseded physical-properties schema") {
    const auto legacy = loadUnitCollectionJson(R"({
        "schema_version": 1,
        "units": [{
            "id": "u",
            "name": "Unit",
            "type": "ship",
            "environment": "water",
            "physical": {
                "mass_kg": 1000,
                "length_m": 20,
                "width_m": 5
            },
            "components": []
        }]
    })");

    CHECK(hasValidationErrors(legacy));
}

TEST_CASE("Unit collection loader rejects the superseded flat fuel schema") {
    const auto legacy = loadUnitCollectionJson(R"({
        "schema_version": 1,
        "units": [{
            "id": "u",
            "name": "Unit",
            "type": "ship",
            "environment": "water",
            "max_speed_knots": 10,
            "fuel_capacity": 100,
            "fuel_reference_speed_knots": 5,
            "fuel_consumption_per_hour": 2,
            "components": []
        }]
    })");

    CHECK(hasValidationErrors(legacy));
}

TEST_CASE("Unit collection loader rejects invalid environments and maximum speeds") {
    CHECK(hasValidationErrors(loadUnitCollectionJson(singleUnitJson("space"))));

    for (const double invalidSpeed : {-1.0}) {
        auto json = nlohmann::json::parse(singleUnitJson());
        json["units"][0]["max_speed_knots"] = invalidSpeed;
        CHECK(hasValidationErrors(loadUnitCollectionJson(json.dump())));
    }
}

TEST_CASE("Unit collection loader rejects malformed fuel values and objects") {
    const auto invalidFuelValues = {
        std::pair{"capacity", 0.0},
        std::pair{"reference_speed_knots", 0.0},
        std::pair{"consumption_per_hour", 0.0}
    };

    for (const auto& [field, value] : invalidFuelValues) {
        auto json = nlohmann::json::parse(singleUnitJson());
        json["units"][0]["fuel"][field] = value;
        CHECK(hasValidationErrors(loadUnitCollectionJson(json.dump())));
    }

    auto missingCapacity = nlohmann::json::parse(singleUnitJson());
    missingCapacity["units"][0]["fuel"].erase("capacity");
    CHECK(hasValidationErrors(loadUnitCollectionJson(missingCapacity.dump())));

    auto wrongFuelType = nlohmann::json::parse(singleUnitJson());
    wrongFuelType["units"][0]["fuel"] = "diesel";
    CHECK(hasValidationErrors(loadUnitCollectionJson(wrongFuelType.dump())));

    auto unknownFuelField = nlohmann::json::parse(singleUnitJson());
    unknownFuelField["units"][0]["fuel"]["extra"] = true;
    CHECK(hasValidationErrors(loadUnitCollectionJson(unknownFuelField.dump())));
}

TEST_CASE("Unit collection loader rejects malformed component references") {
    auto missingId = nlohmann::json::parse(singleUnitJson());
    missingId["units"][0]["components"].push_back({{"quantity", 1}});
    CHECK(hasValidationErrors(loadUnitCollectionJson(missingId.dump())));

    auto zeroQuantity = nlohmann::json::parse(singleUnitJson());
    zeroQuantity["units"][0]["components"].push_back({
        {"component_id", "engine"},
        {"quantity", 0}
    });
    CHECK(hasValidationErrors(loadUnitCollectionJson(zeroQuantity.dump())));

    auto fractionalQuantity = nlohmann::json::parse(singleUnitJson());
    fractionalQuantity["units"][0]["components"].push_back({
        {"component_id", "engine"},
        {"quantity", 1.5}
    });
    CHECK(hasValidationErrors(loadUnitCollectionJson(fractionalQuantity.dump())));
}

TEST_CASE("Unit collection loader rejects duplicate IDs and unknown nested fields") {
    auto duplicate = nlohmann::json::parse(validCollection);
    duplicate["units"][1]["id"] = duplicate["units"][0]["id"];
    CHECK(hasValidationErrors(loadUnitCollectionJson(duplicate.dump())));

    auto extra = nlohmann::json::parse(singleUnitJson());
    extra["units"][0]["unexpected"] = true;
    CHECK(hasValidationErrors(loadUnitCollectionJson(extra.dump())));
}

TEST_CASE("Unit collection loader does not resolve component IDs during parsing") {
    auto json = nlohmann::json::parse(singleUnitJson());
    json["units"][0]["components"].push_back({
        {"component_id", "not_loaded_yet"},
        {"quantity", 1}
    });

    const auto result = loadUnitCollectionJson(json.dump());
    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(result));

    const auto& units = std::get<std::vector<UnitDefinition>>(result);
    REQUIRE(units.size() == 1);
    REQUIRE(units[0].components().size() == 1);
    CHECK(units[0].components()[0].componentId().value() == "not_loaded_yet");
}

TEST_CASE("Unit collection loader reads a collection from a file") {
    const auto path = std::filesystem::temp_directory_path() / "navalsim_unit_collection_test.json";
    {
        std::ofstream file(path);
        REQUIRE(file.is_open());
        file << validCollection;
    }

    const auto result = loadUnitCollectionFile(path);
    std::filesystem::remove(path);

    REQUIRE(std::holds_alternative<std::vector<UnitDefinition>>(result));
    CHECK(std::get<std::vector<UnitDefinition>>(result).size() == 2);
}

TEST_CASE("Unit collection loader reports file access errors") {
    const auto path = std::filesystem::temp_directory_path() / "navalsim_missing_unit_collection.json";
    std::filesystem::remove(path);

    const auto result = loadUnitCollectionFile(path);
    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK((std::get<DataError>(result).code == DataErrorCode::FileOpenFailed));
}
