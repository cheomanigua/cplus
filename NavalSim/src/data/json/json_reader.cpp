
#include "data/json/json_reader.hpp"

#include <fstream>
#include <iterator>
#include <utility>

namespace navalsim::data {

JsonReadResult JsonReader::parse(std::string_view source, std::string sourceName) {
    try {
        auto document = nlohmann::json::parse(source.begin(), source.end());
        return JsonDocument(std::move(document));
    } catch (const nlohmann::json::parse_error& error) {
        return DataError{
            DataErrorCode::JsonParseFailed,
            std::move(sourceName),
            error.what(),
            error.byte,
            true
        };
    }
}

JsonReadResult JsonReader::readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        return DataError{
            DataErrorCode::FileOpenFailed,
            path.string(),
            "Unable to open JSON file."
        };
    }

    std::string contents{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    if (file.bad()) {
        return DataError{
            DataErrorCode::FileReadFailed,
            path.string(),
            "Failed while reading JSON file."
        };
    }

    return parse(contents, path.string());
}

} //namespace

