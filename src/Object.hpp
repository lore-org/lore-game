#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>

#include "Ref.hpp"

#define EVENT_CALLBACK std::function<void(void*)>

class Object : public Ref {
public:
    inline virtual bool isEqual(Object* object) {
        return this == object;
    };
    inline virtual bool operator==(Object* object) {
        return this->isEqual(object);
    };

    virtual void update(float dt) {};

    // event is case-insensitive
    virtual void registerEventListener(std::string name, EVENT_CALLBACK* callback) {
        auto listeners = this->_getOrCreateListeners(name);

        listeners.push_back(callback);
    }
    virtual void unregisterEventListener(std::string name, EVENT_CALLBACK* callback) {
        auto listeners = this->_getOrCreateListeners(name);
        auto find = std::find(listeners.begin(), listeners.end(), callback);

        if (find != listeners.end()) listeners.erase(find);
    };

protected:
    std::unordered_map<std::string, std::vector<EVENT_CALLBACK*>> m_callbacks;

    inline virtual void _callEventListener(std::string name, void* data = nullptr) {
        auto _listeners = m_callbacks.find(name);
        if (_listeners == m_callbacks.end()) return;
        auto listeners = _listeners->second;
        std::for_each(
            listeners.begin(),
            listeners.end(),
            [&data](EVENT_CALLBACK* callback) {
                (*callback)(data);
            }
        );
    };
    
    inline std::vector<EVENT_CALLBACK*> _getOrCreateListeners(std::string name) {
        auto _listeners = m_callbacks.find(name);
        if (_listeners == m_callbacks.end()) m_callbacks.insert(name, {});
        return m_callbacks[name];
    };
};