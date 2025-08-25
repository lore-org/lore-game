#include <engine/Touchable.h>

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

#include <cstdint>

#include <engine/Scheduler.h>
#include <engine/Engine.h>

Touchable::Touchable() : m_isHovered(false), m_lastMouseData(nullptr) {}

bool Touchable::init() {
    if (!Node::init()) return false;

    Scheduler::sharedScheduler()->scheduleUpdate(this, INT64_MIN);
    return true;
}

std::shared_ptr<Touchable> Touchable::create() {
    auto ret = utils::protected_make_shared<Touchable>();

    if (!ret->init()) return nullptr;
    return ret;
}

void Touchable::update(const long double dt) {
    #define IsClicked(button) (!m_lastMouseData->button && mouseData->button)
    #define IsReleased(button) (m_lastMouseData->button && !mouseData->button)

    #define WithAll(method) (method(lmb) || method(mmb) || method(rmb) || method(side1) || method(side2))


    auto rect = this->getRect();
    auto containsLastMouse = rect.containsPoint({ m_lastMouseData->x, m_lastMouseData->y });
    auto mouseData = Engine::getMouseData();

    m_isHovered = rect.containsPoint({ mouseData->x, mouseData->y });

    if (!containsLastMouse && m_isHovered) this->_callEventListener(Events::mouseenter);
    if (!m_isHovered && containsLastMouse) this->_callEventListener(Events::mouseleave);

    if (m_isHovered && WithAll(IsClicked)) this->_callEventListener(Events::mousedown);
    if (WithAll(IsReleased)) this->_callEventListener(Events::mouseup);

    if (m_isHovered && IsReleased(lmb)) this->_callEventListener(Events::click);
    if (m_isHovered && IsReleased(rmb)) this->_callEventListener(Events::context);

    m_lastMouseData = mouseData;

    Object::update(dt);
}