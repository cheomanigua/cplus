#pragma once

#include <string>
#include <utility>
#include <vector>

namespace navalsim::data {

struct ValidationIssue {
    std::string path;
    std::string message;
};

class JsonValidationResult {
public:
    bool valid() const noexcept {
        return issues_.empty();
    }

    const std::vector<ValidationIssue>& issues() const noexcept {
        return issues_;
    }

    void addIssue(std::string path, std::string message) {
        issues_.push_back({std::move(path), std::move(message)});
    }

    void append(const JsonValidationResult& other) {
        issues_.insert(issues_.end(), other.issues_.begin(), other.issues_.end());
    }

private:
    std::vector<ValidationIssue> issues_;

    friend class JsonObjectValidator;
};

}
