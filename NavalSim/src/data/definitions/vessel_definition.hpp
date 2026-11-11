#pragma once

#include <string>
#include <utility>

#include "core/geography/geographic_types.hpp"
#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

class VesselDimensions {
public:
    VesselDimensions(geography::Distance length, geography::Distance beam, geography::Distance draft);

    const geography::Distance& length() const noexcept {
        return length_;
    }

    const geography::Distance& beam() const noexcept {
        return beam_;
    }

    const geography::Distance& draft() const noexcept {
        return draft_;
    }

private:
    geography::Distance length_;
    geography::Distance beam_;
    geography::Distance draft_;
};

class VesselPropulsion {
public:
    explicit VesselPropulsion(double maximumSpeedMetersPerSecond);

    double maximumSpeedMetersPerSecond() const noexcept {
        return maximumSpeedMetersPerSecond_;
    }

private:
    double maximumSpeedMetersPerSecond_;
};

class VesselDefinition {
public:
    VesselDefinition(DefinitionId id, std::string name, VesselDimensions dimensions, VesselPropulsion propulsion);

    const DefinitionId& id() const noexcept {
        return id_;
    }

    const std::string& name() const noexcept {
        return name_;
    }

    const VesselDimensions& dimensions() const noexcept {
        return dimensions_;
    }

    const VesselPropulsion& propulsion() const noexcept {
        return propulsion_;
    }

private:
    DefinitionId id_;
    std::string name_;
    VesselDimensions dimensions_;
    VesselPropulsion propulsion_;
};

}

