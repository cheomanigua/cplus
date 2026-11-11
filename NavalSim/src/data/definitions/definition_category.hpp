#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace navalsim::data {

class DefinitionCategory {
public:
    explicit DefinitionCategory(std::string value) : value_(std::move(value)) {
        if (value_.empty()) throw std::invalid_argument("Definition category cannot be empty.");
    }

    const std::string& value() const noexcept { return value_; }
    bool operator==(const DefinitionCategory&) const = default;

private:
    std::string value_;
};

}
