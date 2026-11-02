#include "flecs.h"
#include <iostream>

// Prefabs are entities that can be used as templates for other entities. They
// are created with a builtin Prefab tag, which by default excludes them from
// queries and systems.

struct Attack { double value{}; };
struct Defense { double value{}; };
struct FreightCapacity { double value{}; };
struct ImpulseSpeed { double value{}; };
struct HasFTL { };
struct Fuel {float value{}; };

int main(int, char *[]) {
    flecs::world ecs;

    flecs::entity spaceship = ecs.prefab("Spaceship")
        .set<ImpulseSpeed>({50})
        .set<Defense>({50});

    flecs::entity freighter = ecs.prefab("Freighter")
        .is_a(spaceship) // inherits all components from spaceship.
        .add<HasFTL>()
        .set<FreightCapacity>({100})
        .set<Defense>({100});

    flecs::entity mammoth_freighter = ecs.prefab("MammothFreighter")
        .is_a(freighter)
        .set<FreightCapacity>({500})
        .set<Defense>({300});

    // Create a regular entity from a prefab.
    flecs::entity inst1 = ecs.entity("my_mammoth_freighter_1")
        .is_a(mammoth_freighter);
    flecs::entity inst2 = ecs.entity("my_mammoth_freighter_2")
        .is_a(mammoth_freighter)
		.set(Defense{400});


    // Inspect the type of the entity.
    std::cout << "Instance type: [" << inst1.type().str() << "]\n";

    // Instance  have a private copy of ImpulseSpeed,
    // we can get it by using the regular API (outputs 50)
    const ImpulseSpeed& ptr = inst1.get<ImpulseSpeed>();
    std::cout << "Impulse speed: " << ptr.value << "\n";
    //std::cout << "Attack: " << inst1.Attack << "\n";
	const Defense& defense = inst1.get<Defense>();
	std::cout << "Defense: " << defense.value << "\n";

    // Prefab components can be iterated just like regular components:
    ecs.each([](flecs::entity e, const Defense& d) {
        std::cout << e.name() << " Defense: " << d.value << "\n";
    });
}
