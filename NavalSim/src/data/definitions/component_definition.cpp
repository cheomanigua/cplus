#include "data/definitions/component_definition.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace navalsim::data {
namespace {

void requirePositiveFinite(double value, const char* fieldName) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(fieldName) + " must be finite and greater than zero.");
    }
}

} // namespace

AaaProperties::AaaProperties(double ratePerMinute, double caliberMm)
    : ratePerMinute_(ratePerMinute), caliberMm_(caliberMm) {
    requirePositiveFinite(ratePerMinute_, "AAA rate per minute");
    requirePositiveFinite(caliberMm_, "AAA caliber");
}

SamProperties::SamProperties(double rangeNm, double speedMetersPerSecond, double warheadKg)
    : rangeNm_(rangeNm), speedMetersPerSecond_(speedMetersPerSecond), warheadKg_(warheadKg) {
    requirePositiveFinite(rangeNm_, "SAM range");
    requirePositiveFinite(speedMetersPerSecond_, "SAM speed");
    requirePositiveFinite(warheadKg_, "SAM warhead mass");
}

EngineProperties::EngineProperties(double thrustN) : thrustN_(thrustN) {
    requirePositiveFinite(thrustN_, "Engine thrust");
}

RadarProperties::RadarProperties(double rangeNm) : rangeNm_(rangeNm) {
    requirePositiveFinite(rangeNm_, "Radar range");
}

ComponentType componentTypeOf(const ComponentProperties& properties) noexcept {
    return std::visit([](const auto& value) noexcept -> ComponentType {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, AaaProperties>) {
            return ComponentType::Aaa;
        } else if constexpr (std::is_same_v<T, SamProperties>) {
            return ComponentType::Sam;
        } else if constexpr (std::is_same_v<T, EngineProperties>) {
            return ComponentType::Engine;
        } else {
            return ComponentType::Radar;
        }
    }, properties);
}

const char* toString(ComponentType type) noexcept {
    switch (type) {
    case ComponentType::Aaa: return "aaa";
    case ComponentType::Sam: return "sam";
    case ComponentType::Engine: return "engine";
    case ComponentType::Radar: return "radar";
    }
    return "unknown";
}

ComponentDefinition::ComponentDefinition(DefinitionId id, std::string name, ComponentProperties properties)
    : id_(std::move(id)), name_(std::move(name)), properties_(std::move(properties)) {
    if (name_.empty()) {
        throw std::invalid_argument("Component definition name cannot be empty.");
    }
}

} // namespace navalsim::data
