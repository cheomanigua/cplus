#include "domains/movement/movement_systems.hpp"

#include <algorithm>
#include <cstddef>

#include "core/geography/geographic_calculations.hpp"
#include "domains/movement/movement_components.hpp"
#include "world/components/tags/entity_type_tags.hpp"

namespace navalsim::movement
{

void registerMovementSystems(flecs::world& world)
{
    // Turning runs first, so movement uses the updated heading.
	world.system<
    Heading,
    DesiredHeading,
    TurnRate,
    TurnDirection,
    navalsim::world::Vessel
	>()
	.kind(flecs::PreUpdate)
	.run(
	    [](flecs::iter& it)
	    {
	        while (it.next())
	        {
	            auto headings = it.field<Heading>(0);
	            auto desiredHeadings = it.field<DesiredHeading>(1);
	            auto turnRates = it.field<TurnRate>(2);
	            auto turnDirections = it.field<TurnDirection>(3);
	
	            const double deltaTime = it.delta_time();
	
	            for (std::size_t i = 0; i < it.count(); ++i)
	            {
	                const double currentHeading = headings[i].degrees();
	                const double desiredHeading =
	                    desiredHeadings[i].degrees();
	
	                // Calculate the shortest signed angular difference.
	                double difference = desiredHeading - currentHeading;
	
	                while (difference <= -180.0)
	                {
	                    difference += 360.0;
	                }
	
	                while (difference > 180.0)
	                {
	                    difference -= 360.0;
	                }
	
	                // Already at the requested heading.
	                if (std::abs(difference) < 1e-9)
	                {
	                    headings[i].setDegrees(desiredHeading);
	                    turnDirections[i] = TurnDirection::None;
	                    continue;
	                }
	
	                // Positive means starboard; negative means port.
	                turnDirections[i] =
	                    difference > 0.0
	                        ? TurnDirection::Starboard
	                        : TurnDirection::Port;
	
	                const double maximumTurn =
	                    turnRates[i].degreesPerSecond() * deltaTime;
	
	                // Never turn beyond the requested heading.
	                const double actualTurn = std::min(
	                    maximumTurn,
	                    std::abs(difference)
	                );
	
	                const double turnSign =
	                    difference > 0.0 ? 1.0 : -1.0;
	
	                headings[i].setDegrees(
	                    currentHeading + (turnSign * actualTurn)
	                );
	
	                // Mark the order complete when the target is reached.
	                if (actualTurn >= std::abs(difference))
	                {
	                    turnDirections[i] = TurnDirection::None;
	                }
	            }
	        }
	    }
	);

    // Movement runs after turning and therefore uses the new heading.
    world.system<
        geography::GeographicPosition,
        Heading,
        Speed,
        MovementStatus,
        navalsim::world::Vessel
    >()
    .kind(flecs::OnUpdate)
    .run(
        [](flecs::iter& it)
        {
            while (it.next())
            {
                auto positions =
                    it.field<geography::GeographicPosition>(0);

                auto headings = it.field<Heading>(1);
                auto speeds = it.field<Speed>(2);
                auto statuses = it.field<MovementStatus>(3);

                const double deltaTime = it.delta_time();

                for (std::size_t i = 0; i < it.count(); ++i)
                {
                    if (statuses[i] != MovementStatus::Underway)
                    {
                        continue;
                    }

                    const double distanceMeters =
                        speeds[i].metersPerSecond() * deltaTime;

                    const geography::Distance distance(distanceMeters);

                    positions[i] = geography::destinationPosition(
                        positions[i],
                        geography::Bearing(headings[i].degrees()),
                        distance
                    );
                }
            }
        }
    );
}

} // namespace navalsim::movement

