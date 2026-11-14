
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/definitions/component_collection_loader.hpp"
#include "data/definitions/definition_loader.hpp"

using namespace navalsim::data;

namespace {

constexpr const char* validCollection = R"({
    "schema_version": 1,
    "components": [
        {
            "id": "marine_engine",
            "name": "Marine Engine",
            "category": "damage_only",
            "type": "engine"
        },
        {
            "id": "propeller",
            "name": "Propeller",
            "category": "damage_only",
            "type": "propeller"
        },
        {
            "id": "radar_search",
            "name": "Search Radar",
            "category": "simulation",
            "type": "radar",
            "properties": {
                "range_nm": 120
            }
        },
        {
            "id": "sam_launcher",
            "name": "SAM Launcher",
            "category": "simulation",
            "type": "sam_launcher"
        }
    ]
})";

bool hasValidationErrors(const ComponentCollectionLoadResult& result) {
    return std::holds_alternative<JsonValidationResult>(result)
        && !std::get<JsonValidationResult>(result).valid();
}

std::string singleRadarJson() {
    return R"({
        "schema_version": 1,
        "components": [{
            "id": "radar_search",
            "name": "Search Radar",
            "category": "simulation",
            "type": "radar",
            "properties": {
                "range_nm": 120
            }
        }]
    })";
}

} // namespace

static_assert(DefinitionCollectionLoader<ComponentCollectionLoader>);

TEST_CASE("Component collection loader converts all component categories and types") {
    const auto result = loadComponentCollectionJson(validCollection);

    REQUIRE(std::holds_alternative<std::vector<ComponentDefinition>>(result));

    const auto& components =
        std::get<std::vector<ComponentDefinition>>(result);

    REQUIRE(components.size() == 4);

    CHECK(components[0].id().value() == "marine_engine");
    CHECK(components[0].name() == "Marine Engine");
    CHECK(
        static_cast<int>(components[0].category()) ==
        static_cast<int>(ComponentCategory::DamageOnly)
    );
    CHECK(components[0].type() == "engine");
    CHECK_FALSE(components[0].properties().has_value());

    CHECK(components[1].id().value() == "propeller");
    CHECK(
        static_cast<int>(components[1].category()) ==
        static_cast<int>(ComponentCategory::DamageOnly)
    );
    CHECK(components[1].type() == "propeller");
    CHECK_FALSE(components[1].properties().has_value());

    CHECK(components[2].id().value() == "radar_search");
    CHECK(
        static_cast<int>(components[2].category()) ==
        static_cast<int>(ComponentCategory::Simulation)
    );
    CHECK(components[2].type() == "radar");
    REQUIRE(components[2].properties().has_value());
    REQUIRE(std::holds_alternative<RadarProperties>(
        *components[2].properties()
    ));
    CHECK(
        std::get<RadarProperties>(*components[2].properties()).rangeNm()
        == doctest::Approx(120.0)
    );

    CHECK(components[3].id().value() == "sam_launcher");
    CHECK(
        static_cast<int>(components[3].category()) ==
        static_cast<int>(ComponentCategory::Simulation)
    );
    CHECK(components[3].type() == "sam_launcher");
    CHECK_FALSE(components[3].properties().has_value());
}

TEST_CASE("Component collection loader accepts an empty collection") {
    const auto result = loadComponentCollectionJson(
        R"({"schema_version":1,"components":[]})"
    );

    REQUIRE(std::holds_alternative<std::vector<ComponentDefinition>>(result));
    CHECK(std::get<std::vector<ComponentDefinition>>(result).empty());
}

TEST_CASE("Component collection loader rejects malformed JSON") {
    const auto result = loadComponentCollectionJson("{ invalid");

    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK(
        std::get<DataError>(result).code == DataErrorCode::JsonParseFailed
    );
}

TEST_CASE("Component collection loader rejects non-object roots and missing collection fields") {
    CHECK(hasValidationErrors(loadComponentCollectionJson("[]")));

    CHECK(hasValidationErrors(
        loadComponentCollectionJson(R"({"components":[]})")
    ));
}

TEST_CASE("Component collection loader rejects unsupported schema versions and unknown root fields") {
    CHECK(hasValidationErrors(
        loadComponentCollectionJson(
            R"({"schema_version":2,"components":[]})"
        )
    ));

    CHECK(hasValidationErrors(
        loadComponentCollectionJson(
            R"({"schema_version":1,"components":[],"extra":true})"
        )
    ));
}

TEST_CASE("Component collection loader rejects missing fields and wrong component types") {
    CHECK(hasValidationErrors(loadComponentCollectionJson(
        R"({"schema_version":1,"components":[{"id":"engine"}]})"
    )));

    CHECK(hasValidationErrors(loadComponentCollectionJson(
        R"({"schema_version":1,"components":["not an object"]})"
    )));
}

TEST_CASE("Component collection loader rejects empty IDs, names, and types") {
    auto emptyId = nlohmann::json::parse(validCollection);
    emptyId["components"][0]["id"] = "";
    CHECK(hasValidationErrors(loadComponentCollectionJson(emptyId.dump())));

    auto emptyName = nlohmann::json::parse(validCollection);
    emptyName["components"][0]["name"] = "";
    CHECK(hasValidationErrors(loadComponentCollectionJson(emptyName.dump())));

    auto emptyType = nlohmann::json::parse(validCollection);
    emptyType["components"][0]["type"] = "";
    CHECK(hasValidationErrors(loadComponentCollectionJson(emptyType.dump())));
}

TEST_CASE("Component collection loader rejects unknown categories") {
    auto json = nlohmann::json::parse(validCollection);
    json["components"][0]["category"] = "unknown";

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader rejects radar components outside the simulation category") {
    auto json = nlohmann::json::parse(singleRadarJson());
    json["components"][0]["category"] = "damage_only";
    json["components"][0].erase("properties");

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader rejects properties on damage-only components") {
    auto json = nlohmann::json::parse(validCollection);
    json["components"][0]["properties"] = nlohmann::json::object();

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader requires radar properties") {
    auto json = nlohmann::json::parse(singleRadarJson());
    json["components"][0].erase("properties");

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader requires radar properties to be an object") {
    auto json = nlohmann::json::parse(singleRadarJson());
    json["components"][0]["properties"] = 120;

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader validates radar range") {
    auto zeroRange = nlohmann::json::parse(singleRadarJson());
    zeroRange["components"][0]["properties"]["range_nm"] = 0;
    CHECK(hasValidationErrors(loadComponentCollectionJson(zeroRange.dump())));

    auto negativeRange = nlohmann::json::parse(singleRadarJson());
    negativeRange["components"][0]["properties"]["range_nm"] = -1;
    CHECK(hasValidationErrors(loadComponentCollectionJson(negativeRange.dump())));

    auto stringRange = nlohmann::json::parse(singleRadarJson());
    stringRange["components"][0]["properties"]["range_nm"] = "120";
    CHECK(hasValidationErrors(loadComponentCollectionJson(stringRange.dump())));

    auto missingRange = nlohmann::json::parse(singleRadarJson());
    missingRange["components"][0]["properties"].erase("range_nm");
    CHECK(hasValidationErrors(loadComponentCollectionJson(missingRange.dump())));
}

TEST_CASE("Component collection loader rejects unknown radar properties") {
    auto json = nlohmann::json::parse(singleRadarJson());
    json["components"][0]["properties"]["unexpected"] = true;

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader rejects properties for unsupported simulation types") {
    auto json = nlohmann::json::parse(validCollection);
    json["components"][3]["properties"] = {
        {"range_nm", 50}
    };

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader rejects unknown component fields") {
    auto json = nlohmann::json::parse(validCollection);
    json["components"][0]["unexpected"] = true;

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader rejects duplicate IDs") {
    auto json = nlohmann::json::parse(validCollection);
    json["components"][1]["id"] = json["components"][0]["id"];

    CHECK(hasValidationErrors(loadComponentCollectionJson(json.dump())));
}

TEST_CASE("Component collection loader reads a collection from a file") {
    const auto path =
        std::filesystem::temp_directory_path()
        / "navalsim_component_collection_test.json";

    {
        std::ofstream file(path);
        REQUIRE(file.is_open());
        file << validCollection;
        REQUIRE(file.good());
    }

    const auto result = loadComponentCollectionFile(path);
    std::filesystem::remove(path);

    REQUIRE(std::holds_alternative<std::vector<ComponentDefinition>>(result));
    CHECK(std::get<std::vector<ComponentDefinition>>(result).size() == 4);
}

TEST_CASE("Component collection loader reports file access errors") {
    const auto path =
        std::filesystem::temp_directory_path()
        / "navalsim_missing_component_collection.json";

    std::filesystem::remove(path);

    const auto result = loadComponentCollectionFile(path);

    REQUIRE(std::holds_alternative<DataError>(result));
    CHECK(
        std::get<DataError>(result).code == DataErrorCode::FileOpenFailed
    );
}

