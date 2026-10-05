#include "text.h"

#include <SDL3/SDL.h>

bool TextKey::operator==(const TextKey& other) const
{
    return text == other.text &&
           color.r == other.color.r &&
           color.g == other.color.g &&
           color.b == other.color.b &&
           color.a == other.color.a;
}

std::size_t TextKeyHash::operator()(const TextKey& key) const
{
    std::size_t hash =
        std::hash<std::string>{}(key.text);

    hash ^=
        static_cast<std::size_t>(key.color.r)
        << 1;

    hash ^=
        static_cast<std::size_t>(key.color.g)
        << 9;

    hash ^=
        static_cast<std::size_t>(key.color.b)
        << 17;

    hash ^=
        static_cast<std::size_t>(key.color.a)
        << 25;

    return hash;
}


TextCache::TextCache(
    SDL_Renderer* renderer,
    TTF_Font* font)
    : renderer_(renderer),
      font_(font)
{
}


TextCache::~TextCache()
{
    Clear();
}


void TextCache::Draw(
    const char* text,
    float x,
    float y,
    SDL_Color color)
{
    const TextKey key{
        text,
        color
    };

    auto iterator = cache_.find(key);

    if (iterator == cache_.end())
    {
        CachedText cached =
            CreateText(key);

        if (!cached.texture)
        {
            return;
        }

        iterator =
            cache_.emplace(
                key,
                cached).first;
    }

    const CachedText& cached =
        iterator->second;

    SDL_FRect destination{
        x,
        y,
        cached.width,
        cached.height
    };

    SDL_RenderTexture(
        renderer_,
        cached.texture,
        nullptr,
        &destination);
}


void TextCache::Clear()
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


CachedText TextCache::CreateText(
    const TextKey& key)
{
    SDL_Surface* surface =
        TTF_RenderText_Blended(
            font_,
            key.text.c_str(),
            0,
            key.color);

    if (!surface)
    {
        SDL_Log(
            "TTF_RenderText_Blended failed: %s",
            SDL_GetError());

        return {};
    }

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(
            renderer_,
            surface);

    if (!texture)
    {
        SDL_Log(
            "SDL_CreateTextureFromSurface failed: %s",
            SDL_GetError());

        SDL_DestroySurface(surface);

        return {};
    }

    CachedText result{
        texture,
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };

    SDL_DestroySurface(surface);

    return result;
}
