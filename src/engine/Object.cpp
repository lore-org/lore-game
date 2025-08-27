#include <engine/Object.h>

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

#include <algorithm>
#include <memory>
#include <string>

#include <engine/Scheduler.h>

Object::Object() {}

bool Object::init() {
    return true;
}

bool Object::init(std::shared_ptr<Update_Callback> update) {
    this->m_updateCallback = update;
    return true;
}

std::shared_ptr<Object> Object::create() {
    auto ret = utils::protected_make_shared<Object>();

    if (!ret->init()) return nullptr;
    return ret;
}

std::shared_ptr<Object> Object::createWithUpdate(std::shared_ptr<Update_Callback> update) {
    auto ret = utils::protected_make_shared<Object>();

    if (!ret->init(update)) return nullptr;
    return ret;
}

void Object::setUpdate(std::shared_ptr<Update_Callback> update) {
    m_updateCallback = update;
}

void Object::scheduleSelf() {
    Scheduler::sharedScheduler()->scheduleUpdate(this);
}

void Object::unscheduleSelf() {
    Scheduler::sharedScheduler()->unscheduleUpdate(this);
}

void Object::update(const long double dt) {
    if (m_updateCallback) (*m_updateCallback)(dt);
}

void Object::registerEventListener(std::string name, std::shared_ptr<Event_Callback> callback) {
    auto listeners = this->_getOrCreateListeners(name);

    listeners.push_back(callback);
}

void Object::unregisterEventListener(std::string name, std::shared_ptr<Event_Callback> callback) {
    auto listeners = this->_getOrCreateListeners(name);
    auto find = std::ranges::find(listeners, callback);

    if (find != listeners.end()) listeners.erase(find);
}

void Object::_callEventListener(std::string name, std::shared_ptr<void> data) {
    auto _listeners = m_callbacks.find(utils::toLowerCase(name));
    if (_listeners == m_callbacks.end()) return;
    auto listeners = _listeners->second;
    std::ranges::for_each(
        listeners,
        [&data](std::shared_ptr<Event_Callback> callback) {
            (*callback)(data);
        }
    );
}

std::vector<std::shared_ptr<Event_Callback>> Object::_getOrCreateListeners(std::string name) {
    auto _listeners = m_callbacks.find(name);
    if (_listeners == m_callbacks.end()) m_callbacks[name] = {};
    return m_callbacks[name];
}