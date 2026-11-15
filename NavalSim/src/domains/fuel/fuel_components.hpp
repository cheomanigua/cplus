#pragma once

namespace navalsim::fuel
{

struct FuelState
{
    double capacity;
    double quantity;
    double referenceSpeedKnots;
    double consumptionPerHour;
};

} // namespace navalsim::fuel
