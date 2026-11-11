#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace navalsim::data {

class DefinitionId {
public:
    explicit DefinitionId(std::string value) : value_(std::move(value)) {
        if (value_.empty()) {
            throw std::invalid_argument("Definition ID cannot be empty.");
        }
    }

    const std::string& value() const noexcept {
        return value_;
    }

    bool operator==(const DefinitionId&) const = default;

private:
    std::string value_;
};

}

