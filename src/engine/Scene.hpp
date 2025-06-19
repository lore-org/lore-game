#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include "Node.hpp"

class Scene : public Node {
public:
    Scene() {}

    virtual bool init() override {
        if (!Node::init()) return false;

        this->setContentSize(Size(GetScreenWidth(), GetScreenHeight()));
        return true;
    }

    static Scene* create() {
        auto ret = new Scene();
        if (!ret->init()) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }
};