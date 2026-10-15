#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <algorithm>

namespace wp
{
    // -----------------------------------------------------------------
    // Types & Structs
    // -----------------------------------------------------------------

    struct Input
    {
        bool left {false};
        bool right {false};
        bool up {false};
        bool down {false};
    };

    struct Circle
    {
        SDL_FPoint position{};
        float radius{};
    };

    struct Rectangle
    {
        SDL_FPoint position{};
        SDL_FPoint size{};

        SDL_FRect GetBounds() const
        {
            return {position.x, position.y, size.x, size.y};
        }
    };

    struct Sprite
    {
        SDL_FPoint position{};
        SDL_Texture* texture {nullptr};

        Sprite(SDL_FPoint position, SDL_Texture* texture)
            : position(position), texture(texture)
        {
        }

        SDL_FRect GetBounds() const
        {
            if (!texture) return {};
            float w{}, h{};
            SDL_GetTextureSize(texture, &w, &h);
            return {position.x, position.y, w, h};
        }
    };

    // -----------------------------------------------------------------
    // Text Caching (from inspirational source text.h / text.cpp)
    // -----------------------------------------------------------------

    struct TextKey
    {
        std::string text;
        SDL_Color color{};

        bool operator==(const TextKey& other) const
        {
            return text == other.text &&
                   color.r == other.color.r &&
                   color.g == other.color.g &&
                   color.b == other.color.b &&
                   color.a == other.color.a;
        }
    };

    struct TextKeyHash
    {
        std::size_t operator()(const TextKey& key) const
        {
            std::size_t hash = std::hash<std::string>{}(key.text);
            hash ^= static_cast<std::size_t>(key.color.r) << 1;
            hash ^= static_cast<std::size_t>(key.color.g) << 9;
            hash ^= static_cast<std::size_t>(key.color.b) << 17;
            hash ^= static_cast<std::size_t>(key.color.a) << 25;

            return hash;
        }
    };

    struct CachedText
    {
        SDL_Texture* texture = nullptr;
        float width = 0.0f;
        float height = 0.0f;
    };

    class TextCache
    {
    public:
        TextCache(SDL_Renderer* renderer, TTF_Font* font)
            : renderer_(renderer), font_(font)
        {
        }

        ~TextCache()
        {
            Clear();
        }

        TextCache(const TextCache&) = delete;
        TextCache& operator=(const TextCache&) = delete;

        void Draw(const char* text, float x, float y, SDL_Color color)
        {
            if (!font_ || !renderer_) return;

            const TextKey key{text, color};
            auto iterator = cache_.find(key);

            if (iterator == cache_.end())
            {
                CachedText cached = CreateText(key);

                if (!cached.texture)
                {
                    return;
                }

                iterator = cache_.emplace(key, cached).first;
            }

            const CachedText& cached = iterator->second;

            SDL_FRect destination{x, y, cached.width, cached.height};
            SDL_RenderTexture(renderer_, cached.texture, nullptr, &destination);
        }

        void Clear()
        {
            for (auto& [key, cached] : cache_)
            {
                if (cached.texture)
                {
                    SDL_DestroyTexture(cached.texture);
                }
            }
            cache_.clear();
        }

    private:
        CachedText CreateText(const TextKey& key)
        {
            SDL_Surface* surface = TTF_RenderText_Blended(font_, key.text.c_str(), 0, key.color);

            if (!surface)
            {
                SDL_Log("TTF_RenderText_Blended failed: %s", SDL_GetError());
                return {};
            }

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

            if (!texture)
            {
                SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                SDL_DestroySurface(surface);
                return {};
            }

            CachedText result{texture, static_cast<float>(surface->w), static_cast<float>(surface->h)};

            SDL_DestroySurface(surface);
            return result;
        }

        SDL_Renderer* renderer_;
        TTF_Font* font_;
        std::unordered_map<TextKey, CachedText, TextKeyHash> cache_;
    };


    // -----------------------------------------------------------------
    // Texture Caching
    // -----------------------------------------------------------------
    
    class TextureCache
    {
    public:
        explicit TextureCache(SDL_Renderer* renderer)
            : renderer_(renderer)
        {
        }
    
        ~TextureCache()
        {
            Clear();
        }
    
        TextureCache(const TextureCache&) = delete;
        TextureCache& operator=(const TextureCache&) = delete;
    
        SDL_Texture* Get(const char* path)
        {
            if (!renderer_ || !path)
                return nullptr;
    
            auto it = textures_.find(path);
    
            if (it != textures_.end())
                return it->second;
    
            SDL_Texture* texture = IMG_LoadTexture(renderer_, path);
    
            if (!texture)
            {
                SDL_Log(
                    "Failed to load texture '%s': %s",
                    path,
                    SDL_GetError()
                );
                return nullptr;
            }
    
            textures_.emplace(path, texture);
            return texture;
        }
    
        void Clear()
        {
            for (auto& [path, texture] : textures_)
            {
                if (texture)
                    SDL_DestroyTexture(texture);
            }
    
            textures_.clear();
        }
    
    private:
        SDL_Renderer* renderer_;
        std::unordered_map<std::string, SDL_Texture*> textures_;
    };

    // -----------------------------------------------------------------
    // Global Context (Internal Timing & State)
    // -----------------------------------------------------------------
    inline struct Context
    {
        SDL_Window* window {nullptr};
        SDL_Renderer* renderer {nullptr};
        TTF_Font* font {nullptr};
        TextCache* textCache {nullptr};
        TextureCache* textureCache {nullptr};
        bool running {false};

        const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
        Uint64 previousCounter {0};
        Uint64 frameStart {0};
        float deltaTime {0.0f};

        const float maxDeltaTime {0.1f};
        const double targetFrameTime {1.0 / 60.0};

        ~Context()
        {
            delete textCache;
            delete textureCache;
            if (font) TTF_CloseFont(font);
            if (renderer) SDL_DestroyRenderer(renderer);
            if (window) SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
        }
    } ctx;

    // -----------------------------------------------------------------
    // Core Lifecycle & Timing API
    // -----------------------------------------------------------------

    inline bool Init(const char* title, int width, int height, const char* fontPath = nullptr, float fontSize = 20.0f)
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) return false;
        if (!TTF_Init())
        {
            SDL_Quit();
            return false;
        }

        if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &ctx.window, &ctx.renderer))
        {
            TTF_Quit();
            SDL_Quit();
            return false;
        }

        ctx.textureCache = new TextureCache(ctx.renderer);

        if (fontPath)
        {
            ctx.font = TTF_OpenFont(fontPath, fontSize);
            if (ctx.font)
            {
                ctx.textCache = new TextCache(ctx.renderer, ctx.font);
            }
        }

        ctx.running = true;
        ctx.previousCounter = SDL_GetPerformanceCounter();
        return true;
    }

    inline SDL_Texture* load(const char* path)
    {
        if (!ctx.textureCache)
            return nullptr;

        return ctx.textureCache->Get(path);
    }

    inline bool IsRunning()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) ctx.running = false;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) ctx.running = false;
        }
        return ctx.running;
    }

    inline void BeginFrame(SDL_Color clearColor = {245, 245, 245, 255})
    {
        ctx.frameStart = SDL_GetPerformanceCounter();
        double rawDeltaTime = static_cast<double>(ctx.frameStart - ctx.previousCounter) / static_cast<double>(ctx.performanceFrequency);
        ctx.previousCounter = ctx.frameStart;

        if (rawDeltaTime > ctx.maxDeltaTime) rawDeltaTime = ctx.maxDeltaTime;
        ctx.deltaTime = static_cast<float>(rawDeltaTime);

        SDL_SetRenderDrawColor(ctx.renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(ctx.renderer);
    }

    inline void EndFrame()
    {
        SDL_RenderPresent(ctx.renderer);

        const Uint64 frameEnd = SDL_GetPerformanceCounter();
        const double frameTime = static_cast<double>(frameEnd - ctx.frameStart) / static_cast<double>(ctx.performanceFrequency);
        const double remaining = ctx.targetFrameTime - frameTime;

        if (remaining > 0.0)
        {
            const Uint32 delayMilliseconds = static_cast<Uint32>(remaining * 1000.0);
            if (delayMilliseconds > 0) SDL_Delay(delayMilliseconds);
        }
    }

    inline float GetDeltaTime() { return ctx.deltaTime; }

    // -----------------------------------------------------------------
    // Input API
    // -----------------------------------------------------------------

    inline Input GetInput()
    {
        const bool* keyboard = SDL_GetKeyboardState(nullptr);

        return {
            keyboard[SDL_SCANCODE_LEFT],
            keyboard[SDL_SCANCODE_RIGHT],
            keyboard[SDL_SCANCODE_UP],
            keyboard[SDL_SCANCODE_DOWN]
        };
    }

    // -----------------------------------------------------------------
    // Collision Detection API
    // -----------------------------------------------------------------

    inline bool CheckCollision(SDL_FPoint point, SDL_FPoint circleCenter, float circleRadius)
    {
        const float dx {point.x - circleCenter.x};
        const float dy {point.y - circleCenter.y};
        return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
    }

    inline bool CheckCollision(SDL_FPoint point, const SDL_FRect& rectangle)
    {
        return point.x >= rectangle.x &&
               point.x < rectangle.x + rectangle.w &&
               point.y >= rectangle.y &&
               point.y < rectangle.y + rectangle.h;
    }

    inline bool CheckCollision(SDL_FPoint centerA, float radiusA, SDL_FPoint centerB, float radiusB)
    {
        const float dx {centerA.x - centerB.x};
        const float dy {centerA.y - centerB.y};
        const float radiusSum {radiusA + radiusB};
        return (dx * dx + dy * dy) <= (radiusSum * radiusSum);
    }

    inline bool CheckCollision(const SDL_FRect& rectangleA, const SDL_FRect& rectangleB)
    {
        return rectangleA.x < rectangleB.x + rectangleB.w &&
               rectangleA.x + rectangleA.w > rectangleB.x &&
               rectangleA.y < rectangleB.y + rectangleB.h &&
               rectangleA.y + rectangleA.h > rectangleB.y;
    }

    inline bool CheckCollision(SDL_FPoint circleCenter, float circleRadius, const SDL_FRect& rectangle)
    {
        const float closestX = std::clamp(circleCenter.x, rectangle.x, rectangle.x + rectangle.w);
        const float closestY = std::clamp(circleCenter.y, rectangle.y, rectangle.y + rectangle.h);
        const float dx {circleCenter.x - closestX};
        const float dy {circleCenter.y - closestY};
        return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
    }

    inline bool CheckCollision(const Circle& a, const Circle& b)
    {
        return CheckCollision(a.position, a.radius, b.position, b.radius);
    }

    inline bool CheckCollision(const Rectangle& a, const Rectangle& b)
    {
        SDL_FRect rectA = a.GetBounds();
        SDL_FRect rectB = b.GetBounds();
        return CheckCollision(rectA, rectB);
    }

    inline bool CheckCollision(const Circle& circle, const Rectangle& rect)
    {
        return CheckCollision(circle.position, circle.radius, rect.GetBounds());
    }

    inline bool CheckCollision(const Sprite& sprite, const Rectangle& rect)
    {
        return CheckCollision(sprite.GetBounds(), rect.GetBounds());
    }

    inline bool CheckCollision(const Sprite& a, const Sprite& b)
    {
        return CheckCollision(a.GetBounds(), b.GetBounds());
    }

    // -----------------------------------------------------------------
    // Drawing Overloads API
    // -----------------------------------------------------------------

    inline void draw(const Circle& circle, SDL_Color color = {0, 121, 241, 255})
    {
        SDL_SetRenderDrawColor(ctx.renderer, color.r, color.g, color.b, color.a);
        const int cx {static_cast<int>(circle.position.x)};
        const int cy {static_cast<int>(circle.position.y)};
        int x {static_cast<int>(circle.radius)};
        int y {0};
        int error {1 - x};

        while (x >= y)
        {
            SDL_RenderPoint(ctx.renderer, cx + x, cy + y);
            SDL_RenderPoint(ctx.renderer, cx + y, cy + x);
            SDL_RenderPoint(ctx.renderer, cx - y, cy + x);
            SDL_RenderPoint(ctx.renderer, cx - x, cy + y);
            SDL_RenderPoint(ctx.renderer, cx - x, cy - y);
            SDL_RenderPoint(ctx.renderer, cx - y, cy - x);
            SDL_RenderPoint(ctx.renderer, cx + y, cy - x);
            SDL_RenderPoint(ctx.renderer, cx + x, cy - y);

            ++y;
            if (error <= 0) error += 2 * y + 1;
            else { --x; error += 2 * (y - x) + 1; }
        }
    }

    inline void draw(const Rectangle& rect, SDL_Color color = {230, 41, 55, 255})
    {
        SDL_SetRenderDrawColor(ctx.renderer, color.r, color.g, color.b, color.a);
        SDL_FRect frect {rect.GetBounds()};
        SDL_RenderRect(ctx.renderer, &frect);
    }

    inline void drawText(const char* text, float x, float y, SDL_Color color = {80, 80, 80, 255})
    {
        if (ctx.textCache)
        {
            ctx.textCache->Draw(text, x, y, color);
        }
    }

    inline SDL_Texture* loadTexture(const char* path)
    {
        SDL_Texture* texture {IMG_LoadTexture(ctx.renderer, path)};
    
        if (!texture)
            SDL_Log("Failed to load texture '%s': %s", path, SDL_GetError());
    
        return texture;
    }

    inline void draw(const Sprite& sprite)
    {
        if (!ctx.renderer || !sprite.texture) return;
    
        SDL_FRect destination = sprite.GetBounds();
        SDL_RenderTexture(ctx.renderer, sprite.texture, nullptr, &destination);
    }
} // namespace wp
