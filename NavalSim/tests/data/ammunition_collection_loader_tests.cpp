#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/definitions/ammunition_collection_loader.hpp"
#include "data/definitions/definition_loader.hpp"

using namespace navalsim::data;

namespace {

constexpr const char* validCollection = R"({
    "schema_version": 1,
    "ammunition": [
        {
            "id": "aim9_sidewinder",
            "name": "AIM-9 Sidewinder",
            "range": 18,
            "speed": 850,
            "is_guided": true,
            "damage": 80
        },
        {
            "id": "mk82_bomb",
            "name": "Mk 82 General Purpose Bomb",
            "range": 15,
            "speed": 250,
            "is_guided": false,
            "damage": 120
        }
    ]
})";

bool hasValidationErrors(const AmmunitionCollectionLoadResult& result) {
    return std::holds_alternative<JsonValidationResult>(result)
        && !std::get<JsonValidationResult>(result).valid();
}

std::string singleAmmunitionJson() {
    return R"({
        "schema_version": 1,
        "ammunition": [{
            "id": "test_ammunition",
            "name": "Test Ammunition",
            "range": 10,
            "speed": 500,
            "is_guided": true,
            "damage": 50
        }]
    })";
}

} // namespace

static_assert(DefinitionCollectionLoader<AmmunitionCollectionLoader>);

TEST_CASE("Ammunition collection loader converts all definitions in a collection") {
    const auto result = loadAmmunitionCollectionJson(validCollection);

    REQUIRE(std::holds_alternative<std::vector<AmmunitionDefinition>>(result));
    const auto& ammunition =
        std::get<std::vector<AmmunitionDefinition>>(result);

    REQUIRE(ammunition.size() == 2);

    CHECK(ammunition[0].id().value() == "aim9_sidewinder");
    CHECK(ammunition[0].name() == "AIM-9 Sidewinder");
    CHECK(ammunition[0].range() == doctest::Approx(18.0));
    CHECK(ammunition[0].speed() == doctest::Approx(850.0));
    CHECK(ammunition[0].isGuided());
    CHECK(ammunition[0].damage() == doctest::Approx(80.0));

    CHECK(ammunition[1].id().value() == "mk82_bomb");
    CHECK(ammunition[1].name() == "Mk 82 General Purpose Bomb");
    CHECK(ammunition[1].range() == doctest::Approx(15.0));
    CHECK(ammunition[1].speed() == doctest::Approx(250.0));
    CHECK_FALSE(ammunition[1].isGuided());
    CHECK(ammunition[1].damage() == doctest::Approx(120.0));
}

TEST_CASE("Ammunition collection loader accepts an empty collection") {
    const auto result = loadAmmunitionCollectionJson(
        R"({"schema_version":1,"ammunition":[]})"
    );

    REQUIRE(std::holds_alternative<std::vector<AmmunitionDefinition>>(result));
    CHECK(std::get<std::vector<AmmunitionDefinition>>(result).empty());
}

TEST_CASE("Ammunition collection loader rejects malformed JSON") {
    const auto result = loadAmmunitionCollectionJson("{ invalid");

    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK(
        std::get<DataError>(result).code == DataErrorCode::JsonParseFailed
    );
}

TEST_CASE("Ammunition collection loader rejects non-object roots and missing collection fields") {
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson("[]")));
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(R"({"ammunition":[]})")));
}

TEST_CASE("Ammunition collection loader rejects unsupported schema versions and unknown root fields") {
    CHECK(hasValidationErrors(
        loadAmmunitionCollectionJson(R"({"schema_version":2,"ammunition":[]})")
    ));

    CHECK(hasValidationErrors(
        loadAmmunitionCollectionJson(
            R"({"schema_version":1,"ammunition":[],"extra":true})"
        )
    ));
}

TEST_CASE("Ammunition collection loader rejects missing fields and wrong field types") {
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(
        R"({"schema_version":1,"ammunition":[{"id":"test_ammunition"}]})"
    )));

    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(
        R"({"schema_version":1,"ammunition":["not an object"]})"
    )));

    auto missingGuidance = nlohmann::json::parse(singleAmmunitionJson());
    missingGuidance["ammunition"][0].erase("is_guided");
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(missingGuidance.dump())));

    auto wrongGuidance = nlohmann::json::parse(singleAmmunitionJson());
    wrongGuidance["ammunition"][0]["is_guided"] = "true";
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(wrongGuidance.dump())));
}

TEST_CASE("Ammunition collection loader rejects empty IDs and names") {
    auto emptyId = nlohmann::json::parse(singleAmmunitionJson());
    emptyId["ammunition"][0]["id"] = "";
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(emptyId.dump())));

    auto emptyName = nlohmann::json::parse(singleAmmunitionJson());
    emptyName["ammunition"][0]["name"] = "";
    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(emptyName.dump())));
}

TEST_CASE("Ammunition collection loader rejects non-positive numeric properties") {
    const std::vector<std::string> fields = {
        "range",
        "speed",
        "damage"
    };

    for (const auto& field : fields) {
        auto zeroValue = nlohmann::json::parse(singleAmmunitionJson());
        zeroValue["ammunition"][0][field] = 0;
        CHECK(hasValidationErrors(loadAmmunitionCollectionJson(zeroValue.dump())));

        auto negativeValue = nlohmann::json::parse(singleAmmunitionJson());
        negativeValue["ammunition"][0][field] = -1;
        CHECK(hasValidationErrors(loadAmmunitionCollectionJson(negativeValue.dump())));
    }
}

TEST_CASE("Ammunition collection loader rejects unknown ammunition fields") {
    auto json = nlohmann::json::parse(singleAmmunitionJson());
    json["ammunition"][0]["unexpected"] = true;

    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(json.dump())));
}

TEST_CASE("Ammunition collection loader rejects duplicate IDs") {
    auto json = nlohmann::json::parse(validCollection);
    json["ammunition"][1]["id"] = json["ammunition"][0]["id"];

    CHECK(hasValidationErrors(loadAmmunitionCollectionJson(json.dump())));
}

TEST_CASE("Ammunition collection loader reads a collection from a file") {
    const auto path =
        std::filesystem::temp_directory_path()
        / "navalsim_ammunition_collection_test.json";

    {
        std::ofstream file(path);
        REQUIRE(file.is_open());
        file << validCollection;
        REQUIRE(file.good());
    }

    const auto result = loadAmmunitionCollectionFile(path);
    std::filesystem::remove(path);

    REQUIRE(std::holds_alternative<std::vector<AmmunitionDefinition>>(result));
    CHECK(std::get<std::vector<AmmunitionDefinition>>(result).size() == 2);
}

TEST_CASE("Ammunition collection loader reports file access errors") {
    const auto path =
        std::filesystem::temp_directory_path()
        / "navalsim_missing_ammunition_collection.json";

    std::filesystem::remove(path);

    const auto result = loadAmmunitionCollectionFile(path);

    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK(
        std::get<DataError>(result).code == DataErrorCode::FileOpenFailed
    );
}

