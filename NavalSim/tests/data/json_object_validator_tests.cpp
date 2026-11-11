#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <string>

#include "data/json/json_object_validator.hpp"

using namespace navalsim::data;

TEST_CASE("JSON validator accepts a valid object") {
    const nlohmann::json value = {
        {"name", "Destroyer"},
        {"speed", 30.0},
        {"active", true}
    };

    JsonObjectValidator validator(value);
    validator.requireString("name", true);
    validator.requireNumber("speed", 0.0, 100.0);
    validator.requireBoolean("active");
    validator.optionalString("description");
    validator.allowOnlyFields({"name", "speed", "active", "description"});

    CHECK(validator.result().valid());
}

TEST_CASE("JSON validator reports missing required fields") {
    const nlohmann::json value = {{"name", "Destroyer"}};

    JsonObjectValidator validator(value);
    validator.requireString("name");
    validator.requireNumber("speed");

    const auto result = validator.result();

    REQUIRE_FALSE(result.valid());
    REQUIRE(result.issues().size() == 1);
    CHECK(result.issues()[0].path == "$.speed");
}

TEST_CASE("JSON validator reports incorrect field types") {
    const nlohmann::json value = {
        {"name", 42},
        {"speed", "fast"},
        {"active", "yes"}
    };

    JsonObjectValidator validator(value);
    validator.requireString("name");
    validator.requireNumber("speed");
    validator.requireBoolean("active");

    const auto result = validator.result();

    CHECK_FALSE(result.valid());
    CHECK(result.issues().size() == 3);
}

TEST_CASE("JSON validator enforces numeric ranges") {
    const nlohmann::json value = {
        {"minimum", -5.0},
        {"maximum", 150.0}
    };

    JsonObjectValidator validator(value);
    validator.requireNumber("minimum", 0.0, 100.0);
    validator.requireNumber("maximum", 0.0, 100.0);

    const auto result = validator.result();

    CHECK_FALSE(result.valid());
    CHECK(result.issues().size() == 2);
}

TEST_CASE("JSON validator rejects unexpected fields") {
    const nlohmann::json value = {
        {"name", "Destroyer"},
        {"unknown", "unexpected"}
    };

    JsonObjectValidator validator(value);
    validator.requireString("name");
    validator.allowOnlyFields({"name"});

    const auto result = validator.result();

    REQUIRE_FALSE(result.valid());
    REQUIRE(result.issues().size() == 1);
    CHECK(result.issues()[0].path == "$.unknown");
}

TEST_CASE("JSON validator rejects a non-object root") {
    const nlohmann::json value = nlohmann::json::array({1, 2, 3});

    JsonObjectValidator validator(value);
    validator.requireString("name");

    const auto result = validator.result();

    REQUIRE_FALSE(result.valid());
    REQUIRE(result.issues().size() == 1);
    CHECK(result.issues()[0].path == "$");
}

TEST_CASE("JSON validator rejects empty strings when required") {
    const nlohmann::json value = {{"name", ""}};

    JsonObjectValidator validator(value);
    validator.requireString("name", true);

    const auto result = validator.result();

    REQUIRE_FALSE(result.valid());
    REQUIRE(result.issues().size() == 1);
    CHECK(result.issues()[0].path == "$.name");
}

TEST_CASE("JSON validator checks nested object and array field types") {
    const nlohmann::json value = {
        {"dimensions", nlohmann::json::object()},
        {"weapons", nlohmann::json::array()}
    };

    JsonObjectValidator validator(value);
    validator.requireObject("dimensions");
    validator.requireArray("weapons");

    CHECK(validator.result().valid());
}

