#pragma once

#include <cstdint>

#include "Node.hpp"
#include "Scheduler.hpp"

/**
 * adds events:
 * `mouseenter  ---  Mouse entered bounding box of node`
 * `mouseleave  ---  Mouse left bounding box of node`
 * `mousedown  ----  LMB held down on node`
 * `mouseup -------  LMB released from node`
 * `click  --------  LMB pressed on node`
 * `context  ------  RMB pressed on node`
 */
class Touchable : public Node {
public:
    virtual bool init() override {
        if (!Node::init()) return false;

        Scheduler::sharedScheduler()->scheduleUpdate(this, INT32_MIN);
        return true;
    }

    static Touchable* create() {
        auto ret = new Touchable();
        if (!ret->init()) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    virtual void update(float dt) override {
        auto rect = this->getRect();
        auto containsLastMouse = rect.containsPoint(m_lastMousePoint);

        m_isHovered = rect.containsPoint(Point(GetMouseX(), GetMouseY()));

        if (!containsLastMouse && m_isHovered) this->_callEventListener("mouseenter");
        if (!m_isHovered && containsLastMouse) this->_callEventListener("mouseleave");

        if (m_isActive && m_isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) this->_callEventListener("click");

        if (m_isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) this->_callEventListener("mousedown");
        if (m_isActive && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) this->_callEventListener("mouseup");

        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) this->_callEventListener("context");

        m_isActive = (m_isActive ? true : m_isHovered) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        m_lastMousePoint = Point(GetMouseX(), GetMouseY());

        Object::update(dt);
    }
protected:
    bool m_isHovered;
    bool m_isActive;
    Point m_lastMousePoint;
};