#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <variant>

#include "data/json/data_error.hpp"
#include "data/json/json_document.hpp"

namespace navalsim::data {

using JsonReadResult = std::variant<JsonDocument, DataError>;

class JsonReader {
public:
    static JsonReadResult parse(std::string_view source, std::string sourceName = "<memory>");

    static JsonReadResult readFile(const std::filesystem::path& path);
};

} //namespace
