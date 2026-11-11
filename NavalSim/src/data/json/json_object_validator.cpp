#include "data/json/json_object_validator.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace navalsim::data {

JsonObjectValidator::JsonObjectValidator(const nlohmann::json& value, std::string rootPath)
    : value_(value), rootPath_(std::move(rootPath)) {
    if (!value_.is_object()) {
        result_.addIssue(rootPath_, "Expected a JSON object.");
    }
}

std::string JsonObjectValidator::fieldPath(std::string_view field) const {
    return rootPath_ + "." + std::string(field);
}

const nlohmann::json* JsonObjectValidator::findField(std::string_view field, bool required) {
    if (!value_.is_object()) {
        return nullptr;
    }

    const auto iterator = value_.find(std::string(field));

    if (iterator == value_.end()) {
        if (required) {
            result_.addIssue(fieldPath(field), "Required field is missing.");
        }
        return nullptr;
    }

    return &*iterator;
}

void JsonObjectValidator::requireString(std::string_view field, bool nonEmpty) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_string()) {
        result_.addIssue(fieldPath(field), "Expected a string.");
    } else if (nonEmpty && value->get_ref<const std::string&>().empty()) {
        result_.addIssue(fieldPath(field), "String must not be empty.");
    }
}

void JsonObjectValidator::optionalString(std::string_view field, bool nonEmpty) {
    const auto* value = findField(field, false);
    if (value == nullptr) return;

    if (!value->is_string()) {
        result_.addIssue(fieldPath(field), "Expected a string.");
    } else if (nonEmpty && value->get_ref<const std::string&>().empty()) {
        result_.addIssue(fieldPath(field), "String must not be empty.");
    }
}

void JsonObjectValidator::requireNumber(std::string_view field, std::optional<double> minimum, std::optional<double> maximum) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_number()) {
        result_.addIssue(fieldPath(field), "Expected a number.");
        return;
    }

    const double number = value->get<double>();

    if (!std::isfinite(number)) {
        result_.addIssue(fieldPath(field), "Number must be finite.");
        return;
    }

    if (minimum && number < *minimum) {
        result_.addIssue(fieldPath(field), "Number is below the minimum allowed value.");
    }

    if (maximum && number > *maximum) {
        result_.addIssue(fieldPath(field), "Number exceeds the maximum allowed value.");
    }
}

void JsonObjectValidator::requireInteger(std::string_view field) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_number_integer()) {
        result_.addIssue(fieldPath(field), "Expected an integer.");
    }
}

void JsonObjectValidator::requireBoolean(std::string_view field) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_boolean()) {
        result_.addIssue(fieldPath(field), "Expected a boolean.");
    }
}

void JsonObjectValidator::requireObject(std::string_view field) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_object()) {
        result_.addIssue(fieldPath(field), "Expected an object.");
    }
}

void JsonObjectValidator::requireArray(std::string_view field) {
    const auto* value = findField(field, true);
    if (value == nullptr) return;

    if (!value->is_array()) {
        result_.addIssue(fieldPath(field), "Expected an array.");
    }
}

void JsonObjectValidator::allowOnlyFields(std::initializer_list<std::string_view> fields) {
    if (!value_.is_object()) return;

    for (auto iterator = value_.begin(); iterator != value_.end(); ++iterator) {
        const bool allowed = std::find(fields.begin(), fields.end(), iterator.key()) != fields.end();

        if (!allowed) {
            result_.addIssue(fieldPath(iterator.key()), "Unexpected field.");
        }
    }
}

JsonValidationResult JsonObjectValidator::result() const {
    return result_;
}

} // namespace
