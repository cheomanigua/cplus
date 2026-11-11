#pragma once

#include <concepts>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "data/json/data_error.hpp"
#include "data/json/json_document.hpp"
#include "data/json/json_reader.hpp"
#include "data/json/json_validation_result.hpp"

namespace navalsim::data {

template<typename Loader>
concept DefinitionLoader = requires(const JsonDocument& document) {
    typename Loader::DefinitionType;
    requires std::same_as<
        decltype(Loader::load(document)),
        std::variant<typename Loader::DefinitionType, JsonValidationResult>
    >;
};

template<DefinitionLoader Loader>
using DefinitionLoadResult = std::variant<
    typename Loader::DefinitionType,
    DataError,
    JsonValidationResult
>;

template<DefinitionLoader Loader>
DefinitionLoadResult<Loader> loadDefinitionDocument(const JsonDocument& document) {
    auto result = Loader::load(document);

    if (auto* definition = std::get_if<typename Loader::DefinitionType>(&result)) {
        return std::move(*definition);
    }

    return std::move(std::get<JsonValidationResult>(result));
}

namespace detail {

template<DefinitionLoader Loader>
DefinitionLoadResult<Loader> loadDefinitionReadResult(JsonReadResult readResult) {
    if (auto* error = std::get_if<DataError>(&readResult)) {
        return std::move(*error);
    }

    return loadDefinitionDocument<Loader>(std::get<JsonDocument>(readResult));
}

}

template<DefinitionLoader Loader>
DefinitionLoadResult<Loader> loadDefinitionJson(
    std::string_view json,
    std::string sourceName = "<memory>"
) {
    return detail::loadDefinitionReadResult<Loader>(
        JsonReader::parse(json, std::move(sourceName))
    );
}

template<DefinitionLoader Loader>
DefinitionLoadResult<Loader> loadDefinitionFile(const std::filesystem::path& path) {
    return detail::loadDefinitionReadResult<Loader>(JsonReader::readFile(path));
}

} // namespace
