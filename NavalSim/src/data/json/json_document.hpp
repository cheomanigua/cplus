#pragma once

#include <utility>

#include <nlohmann/json.hpp>

namespace navalsim::data {

class JsonReader;

class JsonDocument {
public:
    const nlohmann::json& value() const noexcept {
        return value_;
    }

private:
    explicit JsonDocument(nlohmann::json value) : value_(std::move(value)) {}

    nlohmann::json value_;

    friend class JsonReader;
};

} //namespace
