#include <engine/Default.h>

#include <engine/Sprite.h>

#include <memory>
#include <regex>

#include <cpr/cpr.h>

#include <engine/Geometry.h>
#include <engine/ColorNode.h>
#include <engine/Engine.h>
#include <engine/utils.hpp>

Sprite::Sprite() {};

Sprite::~Sprite() {
    Node::~Node();
    
    SDL_DestroyTexture(m_texture);
};

bool Sprite::init(SDL_Texture* texture) {
    if (!ColorNode::init()) return false;

    if (!texture) return false;

    this->setContentSize({
        static_cast<float>(texture->w),
        static_cast<float>(texture->h)
    });

    m_texture = texture;

    return true;
};

std::shared_ptr<Sprite> Sprite::create() {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init({})) return nullptr;
    return ret;
};

std::shared_ptr<Sprite> Sprite::createFromFile(std::string filename) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = IMG_LoadTexture(Engine::sharedInstance()->getRenderer(), filename.c_str());
    if (!tex) {
        PrintSDLError();
        return nullptr;
    };

    if (!ret->init(tex)) return nullptr;
    return ret;
};

std::shared_ptr<Sprite> Sprite::createFromSurface(SDL_Surface* surface) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = SDL_CreateTextureFromSurface(Engine::sharedInstance()->getRenderer(), surface);
    if (!tex) {
        PrintSDLError();
        return nullptr;
    };

    if (!ret->init(tex)) return nullptr;
    return ret;
};

std::shared_ptr<Sprite> Sprite::createFromURL(std::string url) {
    auto ret = utils::protected_make_shared<Sprite>();

    auto tex = Sprite::loadFromURL(url);
    if (!tex) {
        PrintSDLError();
        return nullptr;
    };

    if (!ret->init(tex)) return nullptr;
    return ret;
};

std::shared_ptr<Sprite> Sprite::createFromSprite(std::shared_ptr<Sprite> sprite) {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init(sprite->getTexture())) return nullptr;
    return ret;
};

void Sprite::setTexture(SDL_Texture* texture) {
    SDL_DestroyTexture(m_texture);
    m_texture = texture;
};

SDL_Texture* Sprite::getTexture() {
    return m_texture;
};

void Sprite::draw(const double dt) {
    ColorNode::draw(dt);

    auto renderer = Engine::sharedInstance()->getRenderer();

    auto xOffset = IsZero(m_anchorPoint.x) ? 0 : m_contentSize.width * m_anchorPoint.x;
    auto yOffset = IsZero(m_anchorPoint.y) ? 0 : m_contentSize.height * m_anchorPoint.y;

    if (!SDL_SetRenderDrawColor(
        renderer,
        m_color.r,
        m_color.g,
        m_color.b,
        m_color.a
    )) PrintSDLError();

    SDL_FPoint rotationalAxis = { xOffset, yOffset };
    SDL_FRect destinationRect = {
        m_position.x - xOffset,
        m_position.y - yOffset,
        m_contentSize.width,
        m_contentSize.height
    };

    if (!SDL_RenderTextureRotated(
        renderer,
        m_texture,
        NULL, // Full texture
        &destinationRect,
        m_rotation,
        &rotationalAxis,
        SDL_FLIP_NONE // Todo - implement
    )) PrintSDLError();
};

// May be nullptr, use PrintSDLError()
SDL_Texture* Sprite::loadFromURL(std::string url) {
    auto response = cpr::Get(cpr::Url { url });
    if (response.status_code != 200) {
        PrintError(fmt::format("Image returned status code '{}'", response.status_code));
        return nullptr;
    }
    auto stream = SDL_IOFromConstMem(response.text.data(), response.text.size());
    if (!stream) {
        PrintSDLError();
        return nullptr;
    }

    std::string fileType = {};
    if (response.header.contains("Content-Type")) {
        auto type = response.header.at("Content-Type");
        auto split = utils::splitString(type, std::regex {"/"});

        if (utils::caseInsensitiveCompare(split[1], "jpeg")) split[1] = "jpg";

        if (utils::caseInsensitiveCompare(split[0], "image"))
            fileType = utils::toUpperCase(split[1]).c_str();
        else PrintError("Content-Type is not an Image");
    }

    return IMG_LoadTextureTyped_IO(
        Engine::sharedInstance()->getRenderer(),
        stream, true,
        fileType.empty() ? NULL : fileType.c_str()
    );
};