#pragma once

#include <raylib-cpp.hpp>
#include <fmt/base.h>

#include "Data.hpp"
#include "Node.hpp"
#include "raylib.h"

class RectangleNode : public ColorNode {
public:
    inline virtual bool init(Point origin, Size size) {
        this->setPosition(origin);
        this->setContentSize(size);
        return true;
    }

    static RectangleNode* create() {
        auto ret = new RectangleNode();
        if (!ret->init(0, 0)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    static RectangleNode* createWithVec(Point origin, Size size) {
        auto ret = new RectangleNode();
        if (!ret->init(origin, size)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    static RectangleNode* createWithRect(Rect rectangle) {
        auto ret = new RectangleNode();
        if (!ret->init(rectangle.origin, rectangle.size)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    virtual void draw(float dt) const {
        auto xOffset = IsZero(m_anchorPoint.x) ? 0 : m_contentSize.width * m_anchorPoint.x;
        auto yOffset = IsZero(m_anchorPoint.y) ? 0 : m_contentSize.height * m_anchorPoint.y;
        DrawRectangleV(
            m_position - Point(xOffset, yOffset),
            m_contentSize,
            m_color
        );

        std::for_each(
            m_children.begin(),
            m_children.end(),
            [dt](Node* child) { child->draw(dt); }
        );
    }
};