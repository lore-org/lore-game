#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

#include <memory>

#include "ColorNode.h"

class Sprite : public ColorNode {
public:
    virtual ~Sprite();

    virtual bool init(SDL_Texture* texture);

    static std::shared_ptr<Sprite> create();
    static std::shared_ptr<Sprite> createFromFile(std::string filename);
    static std::shared_ptr<Sprite> createFromSurface(SDL_Surface* surface);
    // Uses a web URL to load an image
    static std::shared_ptr<Sprite> createFromURL(std::string url);
    // Copies the sprite's texture to a new instance
    static std::shared_ptr<Sprite> createFromSprite(std::shared_ptr<Sprite> sprite);

    void setTexture(SDL_Texture* texture);
    SDL_Texture* getTexture();

    virtual void draw(const long double dt) override;

protected:
    Sprite();

    SDL_Texture* m_texture;

    static SDL_Texture* loadFromURL(std::string url);

private:
    using ColorNode::init;
};