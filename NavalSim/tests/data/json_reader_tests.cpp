#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "data/json/json_reader.hpp"

using namespace navalsim::data;

TEST_CASE("JSON reader parses a valid JSON document") {
    const auto result = JsonReader::parse(R"({"name":"Destroyer","speed":30})");

    REQUIRE(std::holds_alternative<JsonDocument>(result));

    const auto& document = std::get<JsonDocument>(result);
    CHECK(document.value().at("name") == "Destroyer");
    CHECK(document.value().at("speed") == 30);
}

TEST_CASE("JSON reader reports malformed JSON") {
    const auto result = JsonReader::parse(R"({"name":})", "test.json");

    REQUIRE(std::holds_alternative<DataError>(result));

    const auto& error = std::get<DataError>(result);
    CHECK(error.code == DataErrorCode::JsonParseFailed);
    CHECK(error.source == "test.json");
    CHECK_FALSE(error.message.empty());
    CHECK(error.hasBytePosition);
}

TEST_CASE("JSON reader reads a JSON file") {
    const auto path = std::filesystem::temp_directory_path() / "navalsim_json_reader_test.json";

    {
        std::ofstream file(path);
        REQUIRE(file.is_open());
        file << R"({"name":"Frigate","displacement":4500})";
        REQUIRE(file.good());
    }

    const auto result = JsonReader::readFile(path);
    std::filesystem::remove(path);

    REQUIRE(std::holds_alternative<JsonDocument>(result));

    const auto& document = std::get<JsonDocument>(result);
    CHECK(document.value().at("name") == "Frigate");
    CHECK(document.value().at("displacement") == 4500);
}

TEST_CASE("JSON reader reports a missing file") {
    const auto path = std::filesystem::temp_directory_path() / "navalsim_missing_json_file.json";
    std::filesystem::remove(path);

    const auto result = JsonReader::readFile(path);

    REQUIRE(std::holds_alternative<DataError>(result));

    const auto& error = std::get<DataError>(result);
    CHECK(error.code == DataErrorCode::FileOpenFailed);
    CHECK(error.source == path.string());
}
