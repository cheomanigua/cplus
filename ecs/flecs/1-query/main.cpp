#include <print>
#include "flecs.h"

struct Position {
	float x{}, y{};
};

void printPos(flecs::query<const Position>& q)
{
	q.each([](flecs::entity e, const Position& pos) {
			std::println("Entity {}: {}, {}", e.id(), pos.x, pos.y);
			});
}

int main() {
	flecs::world world{};
	auto posQuery = world.query_builder<const Position>().cached().build();

	for(std::size_t i{0} ; i < 5; ++i) {
		float value {static_cast<float>(i)};
		world.entity()
			.set(Position{value, value});
	}
	printPos(posQuery);

	return 0;
}
