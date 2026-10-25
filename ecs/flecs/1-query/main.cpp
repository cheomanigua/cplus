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
		auto  entity = world.entity();
		entity.set<Position>({static_cast<float>(entity.id()), static_cast<float>(entity.id())});
	}
	printPos(posQuery);

	return 0;
}
