#pragma once
#include "Ref.hpp"


class Object : public Ref {
public:
    virtual bool isEqual(Object* object) {
        return this == object;
    };
    virtual bool operator==(Object* object) {
        return this->isEqual(object);
    };

    virtual void update(float dt) {};
};