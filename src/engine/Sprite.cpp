#include <engine/Sprite.h>

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
#include <regex>

#include <cpr/cpr.h>

#include <engine/Geometry.h>
#include <engine/ColorNode.h>
#include <engine/Engine.h>

Sprite::Sprite() {}

Sprite::~Sprite() {
    Node::~Node();
    
    SDL_DestroyTexture(m_texture);
}

bool Sprite::init(SDL_Texture* texture) {
    if (!ColorNode::init()) return false;

    if (!texture) return false;

    this->setContentSize({
        static_cast<double>(texture->w),
        static_cast<double>(texture->h)
    });

    m_texture = texture;

    return true;
}

std::shared_ptr<Sprite> Sprite::create() {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init({})) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromFile(std::string filename) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = IMG_LoadTexture(Engine::sharedInstance()->getRenderer(), filename.c_str());
    if (!tex) {
        LogSDLError();
        return nullptr;
    }

    if (!ret->init(tex)) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromSurface(SDL_Surface* surface) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = SDL_CreateTextureFromSurface(Engine::sharedInstance()->getRenderer(), surface);
    if (!tex) {
        LogSDLError();
        return nullptr;
    }

    if (!ret->init(tex)) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromURL(std::string url) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = Sprite::loadFromURL(url);
    if (!tex) {
        LogSDLError();
        return nullptr;
    }

    if (!ret->init(tex)) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromSprite(std::shared_ptr<Sprite> sprite) {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init(sprite->getTexture())) return nullptr;
    return ret;
}

void Sprite::setTexture(SDL_Texture* texture) {
    SDL_DestroyTexture(m_texture);
    m_texture = texture;
}

SDL_Texture* Sprite::getTexture() {
    return m_texture;
}

void Sprite::draw(const double dt) {
    ColorNode::draw(dt);

    auto renderer = Engine::sharedInstance()->getRenderer();

    auto scaledWidth = m_contentSize.width * m_scale;
    auto scaledHeight = m_contentSize.height * m_scale;

    auto xOffset = IsZero(m_anchorPoint.x) ? 0 : scaledWidth * m_anchorPoint.x;
    auto yOffset = IsZero(m_anchorPoint.y) ? 0 : scaledHeight * m_anchorPoint.y;

    if (!SDL_SetTextureColorMod(
        m_texture,
        m_color.r,
        m_color.g,
        m_color.b
    )) LogSDLError();
    if (!SDL_SetTextureAlphaMod(
        m_texture,
        m_color.a
    )) LogSDLError();
    if (!SDL_SetRenderDrawBlendMode(
        renderer,
        m_blendMode
    )) LogSDLError();

    SDL_FPoint rotationalAxis = { static_cast<float>(xOffset), static_cast<float>(yOffset) };
    SDL_FRect destinationRect = {
        static_cast<float>(m_position.x - xOffset),
        static_cast<float>(m_position.y - yOffset),
        static_cast<float>(scaledWidth),
        static_cast<float>(scaledHeight)
    };

    if (!SDL_RenderTextureRotated(
        renderer,
        m_texture,
        nullptr, // Full texture
        &destinationRect,
        m_rotation,
        &rotationalAxis,
        SDL_FLIP_NONE // TODO - implement
    )) LogSDLError();
}

// May be nullptr, use LogSDLError()
SDL_Texture* Sprite::loadFromURL(std::string url) {
    auto response = cpr::Get(cpr::Url { url });
    if (response.status_code != 200) {
        LogError(fmt::format("Image returned status code '{}'", response.status_code));
        return nullptr;
    }
    auto stream = SDL_IOFromConstMem(response.text.data(), response.text.size());
    if (!stream) {
        LogSDLError();
        return nullptr;
    }

    std::string fileType = {};
    if (response.header.contains("Content-Type")) {
        auto type = response.header.at("Content-Type");
        auto split = utils::splitString(type, std::regex {"/"});

        if (utils::caseInsensitiveCompare(split[1], "jpeg")) split[1] = "jpg";

        if (utils::caseInsensitiveCompare(split[0], "image"))
            fileType = utils::toUpperCase(split[1]).c_str();
        else LogError("Content-Type is not an Image");
    }

    return IMG_LoadTextureTyped_IO(
        Engine::sharedInstance()->getRenderer(),
        stream, true,
        fileType.empty() ? nullptr : fileType.c_str()
    );
}