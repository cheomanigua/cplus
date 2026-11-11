#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "flecs.h"

struct Position {
    SDL_FPoint value{};
};

struct Velocity {
    SDL_FPoint value{};
};

struct Sprite {
    SDL_Texture* texture{};
    SDL_FPoint size{};
};

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("My Window", 800, 600, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* surface_blue = IMG_LoadTexture(renderer, "assets/surface_blue.svg");
    SDL_Texture* submarine_blue = IMG_LoadTexture(renderer, "assets/submarine_blue.svg");
    SDL_Texture* plane_blue = IMG_LoadTexture(renderer, "assets/plane_blue.svg");
    SDL_Texture* carrier_red = IMG_LoadTexture(renderer, "assets/carrier_red.svg");

    if (!surface_blue || !submarine_blue || !plane_blue || !carrier_red) {
        SDL_Log("IMG_LoadTexture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

	// Set FPS to monitor refresh rate
	SDL_SetRenderVSync(renderer, 1);

    float surfaceW{}, submarineW{}, planeW{}, carrierW{};
    float surfaceH{}, submarineH{}, planeH{}, carrierH{};

    SDL_GetTextureSize(surface_blue, &surfaceW, &surfaceH);
    SDL_GetTextureSize(submarine_blue, &submarineW, &submarineH);
    SDL_GetTextureSize(plane_blue, &planeW, &planeH);
    SDL_GetTextureSize(carrier_red, &carrierW, &carrierH);

    flecs::world world;

    world.entity("Surface")
        .set<Position>({100.0f, 100.0f})
        .set<Velocity>({3.0f, 0.0f})
        .set<Sprite>({surface_blue, {surfaceW, surfaceH}});

    world.entity("Submarine")
        .set<Position>({200.0f, 200.0f})
        .set<Velocity>({0.0f, 1.5f})
        .set<Sprite>({submarine_blue, {submarineW, submarineH}});

    world.entity("Plane")
        .set<Position>({250.0f, 200.0f})
        .set<Velocity>({70.0f, 70.0f})
        .set<Sprite>({plane_blue, {planeW, planeH}});

    world.entity("Carrier")
        .set<Position>({250.0f, 200.0f})
        .set<Velocity>({0.0f, -3.0f})
        .set<Sprite>({carrier_red, {carrierW, carrierH}});

    float deltaTime {};

    world.system<Position, const Velocity>("Movement")
        .each([&](Position& p, const Velocity& v) {
            p.value.x += (v.value.x) * deltaTime;
            p.value.y += (v.value.y) * deltaTime;
        });

	auto drawQuery = world.query_builder<const Position, const Sprite>()
    .cached()
    .build();

    bool done = false;
    Uint64 previousTicks = SDL_GetTicks();

    while (!done) {
		SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                done = true;
            }
        }

        Uint64 currentTicks = SDL_GetTicks();
        deltaTime = (currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;

        // Update game state through Flecs.
        world.progress(deltaTime);

        // Render through SDL3.
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_RenderClear(renderer);

		drawQuery.each(
		[&](const Position& p, const Sprite& s) {
			SDL_FRect spriteRect = {
        	    p.value.x,
        	    p.value.y,
        	    s.size.x,
        	    s.size.y
        	};

        	SDL_RenderTexture(renderer, s.texture, nullptr, &spriteRect);
		});

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(carrier_red);
    SDL_DestroyTexture(surface_blue);
    SDL_DestroyTexture(submarine_blue);
    SDL_DestroyTexture(plane_blue);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
