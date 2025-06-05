#pragma once

#include <algorithm>
#include <functional>
#include <string>

#include "Ref.hpp"

#define EVENT_CALLBACK std::function<void(void*)>

struct Event {
    std::string name;
    EVENT_CALLBACK* callback;

    inline bool operator==(const Event& other) {
        return this->name == other.name && this->callback == other.callback;
    }
};

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
    virtual Event registerEventListener(std::string name, EVENT_CALLBACK callback) {
        Event event = { name, &callback };

        m_callbacks.push_back(event);
        return event;
    }
    virtual void unregisterEventListener(Event event) {
        auto find = std::find(m_callbacks.begin(), m_callbacks.end(), event);

        if (find != m_callbacks.end()) m_callbacks.erase(find);
    };

protected:
    std::vector<Event> m_callbacks;

private:
    inline virtual void _callEventListener(std::string name, void* data = nullptr) {
        std::for_each(
            m_callbacks.begin(),
            m_callbacks.end(),
            [name, &data](Event event) {
                if (event.name == name) (*event.callback)(data);
            }
        );
    };

    friend class Node;
};