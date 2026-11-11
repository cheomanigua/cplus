#pragma once

#include <string>
#include <variant>

#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

class AaaProperties {
public:
    AaaProperties(double ratePerMinute, double caliberMm);
    double ratePerMinute() const noexcept { return ratePerMinute_; }
    double caliberMm() const noexcept { return caliberMm_; }

private:
    double ratePerMinute_;
    double caliberMm_;
};

class SamProperties {
public:
    SamProperties(double rangeNm, double speedMetersPerSecond, double warheadKg);
    double rangeNm() const noexcept { return rangeNm_; }
    double speedMetersPerSecond() const noexcept { return speedMetersPerSecond_; }
    double warheadKg() const noexcept { return warheadKg_; }

private:
    double rangeNm_;
    double speedMetersPerSecond_;
    double warheadKg_;
};

class EngineProperties {
public:
    explicit EngineProperties(double thrustN);
    double thrustN() const noexcept { return thrustN_; }

private:
    double thrustN_;
};

class RadarProperties {
public:
    explicit RadarProperties(double rangeNm);
    double rangeNm() const noexcept { return rangeNm_; }

private:
    double rangeNm_;
};

using ComponentProperties = std::variant<
    AaaProperties,
    SamProperties,
    EngineProperties,
    RadarProperties
>;

enum class ComponentType {
    Aaa,
    Sam,
    Engine,
    Radar
};

ComponentType componentTypeOf(const ComponentProperties& properties) noexcept;
const char* toString(ComponentType type) noexcept;

class ComponentDefinition {
public:
    ComponentDefinition(DefinitionId id, std::string name, ComponentProperties properties);

    const DefinitionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    ComponentType type() const noexcept { return componentTypeOf(properties_); }
    const ComponentProperties& properties() const noexcept { return properties_; }

private:
    DefinitionId id_;
    std::string name_;
    ComponentProperties properties_;
};

} // namespace navalsim::data
