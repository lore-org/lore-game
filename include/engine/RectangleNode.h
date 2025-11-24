#pragma once

#include <memory>

#include <engine/ColorNode.h>

class RectangleNode : public ColorNode {
public:
    virtual bool init(Point origin, Size size);

    static std::shared_ptr<RectangleNode> create();

    static std::shared_ptr<RectangleNode> createWithVec(Point origin, Size size);

    static std::shared_ptr<RectangleNode> createWithRect(Rect rectangle);

    virtual void draw(const long double dt) override;

protected:
    RectangleNode();

private:
    using ColorNode::init;

    bool m_filled;
};