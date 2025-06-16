#pragma once

#include "Node.hpp"
#include "utils.hpp"

class Scene : public Node {
public:
    Scene() {}

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
    
    inline virtual void draw(float dt) {
        fmt::println("{}", fmt::ptr(m_children[0]));
        std::for_each(
            m_children.begin(),
            m_children.end(),
            [dt](Node* child) { child->draw(dt); }
        );
    };
};