#include "SDL3/SDL_blendmode.h"
#include <engine/Default.h>

#include <engine/RectangleNode.h>

#include <engine/Engine.h>
#include <engine/utils.hpp>

RectangleNode::RectangleNode() : m_filled(true) {};

bool RectangleNode::init(Point origin, Size size) {
    if (!ColorNode::init()) return false;

    this->setPosition(origin);
    this->setContentSize(size);
    return true;
};

std::shared_ptr<RectangleNode> RectangleNode::create() {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(0, 0)) return nullptr;
    return ret;
};

std::shared_ptr<RectangleNode> RectangleNode::createWithVec(Point origin, Size size) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(origin, size)) return nullptr;
    return ret;
};

std::shared_ptr<RectangleNode> RectangleNode::createWithRect(Rect rectangle) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(rectangle.origin, rectangle.size)) return nullptr;
    return ret;
};

void RectangleNode::draw(const double dt) {
    ColorNode::draw(dt);

    auto renderer = Engine::sharedInstance()->getRenderer();

    auto xOffset = IsZero(m_anchorPoint.x) ? 0 : m_contentSize.width * m_anchorPoint.x;
    auto yOffset = IsZero(m_anchorPoint.y) ? 0 : m_contentSize.height * m_anchorPoint.y;

    auto translatedPosition = m_position - Point(xOffset, yOffset);

    if (!SDL_SetRenderDrawColor(
        renderer,
        m_color.r,
        m_color.g,
        m_color.b,
        m_color.a
    )) PrintSDLError();
    if (!SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND // TODO - implement
    )) PrintSDLError();

    SDL_FRect rect = {
        translatedPosition.x,
        translatedPosition.y,
        m_contentSize.width,
        m_contentSize.height
    };

    if (!(m_filled ? SDL_RenderFillRect : SDL_RenderRect)(
        renderer,
        &rect
    )) PrintSDLError();
};