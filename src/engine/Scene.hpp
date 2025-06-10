#pragma once

#include "Node.hpp"

class Scene : public Node {
public:
    inline virtual bool init() {
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