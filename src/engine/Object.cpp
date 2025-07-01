#include <engine/Default.h>

#include <engine/Object.h>

#include <algorithm>
#include <memory>
#include <string>

#include <engine/utils.hpp>
#include <engine/Scheduler.h>

Object::Object() {};

bool Object::isEqual(std::shared_ptr<Object> object) {
    return this == object.get();
}

bool Object::operator==(std::shared_ptr<Object> object) {
    return this->isEqual(object);
}

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

void Object::update(const double dt) {
    if (m_updateCallback) (*m_updateCallback)(dt);
}

void Object::registerEventListener(std::string name, std::shared_ptr<Event_Callback> callback) {
    auto listeners = this->_getOrCreateListeners(name);

    listeners.push_back(callback);
}

void Object::unregisterEventListener(std::string name, std::shared_ptr<Event_Callback> callback) {
    auto listeners = this->_getOrCreateListeners(name);
    auto find = std::find(listeners.begin(), listeners.end(), callback);

    if (find != listeners.end()) listeners.erase(find);
}

void Object::_callEventListener(std::string name, std::shared_ptr<void> data) {
    auto _listeners = m_callbacks.find(utils::toLowerCase(name));
    if (_listeners == m_callbacks.end()) return;
    auto listeners = _listeners->second;
    std::for_each(
        listeners.begin(),
        listeners.end(),
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