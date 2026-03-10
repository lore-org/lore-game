#include <furredengine/Touchable.h>

#include <cstdint>

#include <discord-rpc.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <furredengine/config.hpp>
#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>
#include <furredengine/Scheduler.h>
#include <furredengine/Engine.h>

using namespace FurredEngine;

Touchable::Touchable() : m_isHovered(false), m_isPressed(false), m_isFocused(false), m_lastMouseData(Engine::sharedInstance()->getMouseData()) {}

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
    #define IsHeld(button) (mouseData.button)

    #define IsClicked(button) (!m_lastMouseData.button && mouseData.button)
    #define IsReleased(button) (m_lastMouseData.button && !mouseData.button)

    #define NodeIsClicked(button) (m_isHovered && wasPressed && IsClicked(button))
    #define NodeIsReleased(button) (wasPressed && IsReleased(button))

    #define WithAll(method) (method(lmb) || method(mmb) || method(rmb) || method(side1) || method(side2))

    auto wasHovered = m_isHovered;
    auto wasPressed = m_isPressed;
    auto wasFocused = m_isFocused;

    auto mouseData = Engine::sharedInstance()->getMouseData();

    m_isHovered = this->containsPoint({ mouseData.x, mouseData.y });
    m_isPressed = m_isHovered && WithAll(IsHeld);
    m_isFocused = !(WithAll(IsClicked) && !m_isHovered) && (wasFocused || (!wasPressed && m_isPressed));

    // LogInfo("----------------------------------");
    // LogInfo(fmt::format("wasHovered     {}", wasHovered));
    // LogInfo(fmt::format("m_isHovered    {}", m_isHovered));
    // LogInfo(fmt::format("wasPressed     {}", wasPressed));
    // LogInfo(fmt::format("m_isPressed    {}", m_isPressed));
    // LogInfo(fmt::format("wasFocused     {}", wasFocused));
    // LogInfo(fmt::format("m_isFocused    {}", m_isFocused));

    if (!wasHovered && m_isHovered) this->_callEventListener(Events::mouseenter);
    else if (!m_isHovered && wasHovered) this->_callEventListener(Events::mouseleave);

    if (!wasPressed && m_isPressed) this->_callEventListener(Events::mousedown);
    else if (!m_isPressed && wasPressed) this->_callEventListener(Events::mouseup);

    if (NodeIsReleased(lmb)) this->_callEventListener(Events::click);
    if (NodeIsReleased(mmb)) this->_callEventListener(Events::pick);
    if (NodeIsReleased(rmb)) this->_callEventListener(Events::context);
    if (NodeIsReleased(side1)) this->_callEventListener(Events::forward);
    if (NodeIsReleased(side2)) this->_callEventListener(Events::backward);

    m_lastMouseData = mouseData;

    Object::update(dt);
}