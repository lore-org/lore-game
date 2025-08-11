#include <engine/RectangleNode.h>

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

#include <engine/Engine.h>

RectangleNode::RectangleNode() : m_filled(true) {}

bool RectangleNode::init(Point origin, Size size) {
    if (!ColorNode::init()) return false;

    this->setPosition(origin);
    this->setContentSize(size);
    return true;
}

std::shared_ptr<RectangleNode> RectangleNode::create() {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(0, 0)) return nullptr;
    return ret;
}

std::shared_ptr<RectangleNode> RectangleNode::createWithVec(Point origin, Size size) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(origin, size)) return nullptr;
    return ret;
}

std::shared_ptr<RectangleNode> RectangleNode::createWithRect(Rect rectangle) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(rectangle.origin, rectangle.size)) return nullptr;
    return ret;
}

void RectangleNode::draw(const double dt) {
    ColorNode::draw(dt);

    auto renderer = Engine::sharedInstance()->getRenderer();
    
    auto scaledWidth = m_contentSize.width * m_scale;
    auto scaledHeight = m_contentSize.height * m_scale;

    auto xOffset = IsZero(m_anchorPoint.x) ? 0 : scaledWidth * m_anchorPoint.x;
    auto yOffset = IsZero(m_anchorPoint.y) ? 0 : scaledHeight * m_anchorPoint.y;

    auto translatedPosition = m_position - Point(xOffset, yOffset);

    if (!SDL_SetRenderDrawColor(
        renderer,
        m_color.r,
        m_color.g,
        m_color.b,
        m_color.a
    )) LogSDLError();
    if (!SDL_SetRenderDrawBlendMode(
        renderer,
        m_blendMode
    )) LogSDLError();

    SDL_FRect rect = {
        static_cast<float>(translatedPosition.x),
        static_cast<float>(translatedPosition.y),
        static_cast<float>(scaledWidth),
        static_cast<float>(scaledHeight)
    };

    if (!(m_filled ? SDL_RenderFillRect : SDL_RenderRect)(
        renderer,
        &rect
    )) LogSDLError();
}