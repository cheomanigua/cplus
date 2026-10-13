#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstddef>
#include <string>
#include <unordered_map>

struct TextKey
{
    std::string text;
    SDL_Color color{};

    bool operator==(const TextKey& other) const;
};

struct TextKeyHash
{
    std::size_t operator()(const TextKey& key) const;
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
    TextCache(
        SDL_Renderer* renderer,
        TTF_Font* font);

    ~TextCache();

    TextCache(const TextCache&) = delete;
    TextCache& operator=(const TextCache&) = delete;

    void Draw(
        const char* text,
        float x,
        float y,
        SDL_Color color);

    void Clear();

private:
    CachedText CreateText(const TextKey& key);

    SDL_Renderer* renderer_;
    TTF_Font* font_;

    std::unordered_map<
        TextKey,
        CachedText,
        TextKeyHash> cache_;
};
