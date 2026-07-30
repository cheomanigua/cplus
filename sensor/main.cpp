#include <iostream>

struct PositionComp {
    float X{};
    float Y{};
};

struct SensorComp {
    float RangeSquared{};
    bool IsEnabled{};

    SensorComp(float range, bool isEnabled)
        : RangeSquared(range * range), IsEnabled(isEnabled) {}
};

// C++ equivalent of 'in': const reference (const &)
// Passes by memory address (efficient) and enforces read-only (safety)
bool IsWithinRadarRange(
        const PositionComp& sourcePos,
        const PositionComp& targetPos,
        const SensorComp& radar)
{
    if (!radar.IsEnabled)
        return false;

    float deltaX = targetPos.X - sourcePos.X;
    float deltaY = targetPos.Y - sourcePos.Y;
    float distanceSquared = (deltaX * deltaX) + (deltaY * deltaY);

    std::cout << "Distance: " << distanceSquared 
              << ". Radar Range: " << radar.RangeSquared << "\n";

    return distanceSquared <= radar.RangeSquared;
}

int main() {
    // Stack allocation (standard behavior for structs in C++)
    PositionComp ussPasadenaPos {110.15f, 30.85f};
    PositionComp targetPos {160.14f, 31.15f};
    SensorComp passiveRadar {50.0f, true};

    bool detected { IsWithinRadarRange(ussPasadenaPos, targetPos, passiveRadar) };

    std::cout << "Target Detected: " << (detected ? "True" : "False") << "\n";

    return 0;
}
