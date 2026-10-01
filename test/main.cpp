#include <iostream>
#include <vector>
#include "entt.hpp"

entt::registry reg;

struct Position {
	float x{}, y{};
};

void printPosition(entt::registry& reg)
{
    auto view = reg.view<Position>();

    for (auto [entity, position] : view.each())
    {
        std::cout << "Entity " << entt::to_entity(entity) << ": "
                  << position.x << ", "
                  << position.y << '\n';
    }
	std::cout << "\n";
}

int main() {

	for(std::size_t i = 0; i < 5; ++i)
	{
		auto entity = reg.create();
		reg.emplace<Position>(entity, static_cast<float>(i), static_cast<float>(i));
	};

	printPosition(reg);
	reg.destroy(static_cast<entt::entity>(1));
	printPosition(reg);

	auto entity = reg.create();
	reg.emplace<Position>(entity, 5.0f, 5.0f);
	printPosition(reg);

	return 0;
}
