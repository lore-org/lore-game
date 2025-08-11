#pragma once

#include "Node.h"
#include <engine/Engine.h>
#include <engine/utils.hpp>

class Touchable : public Node {
public:
    struct Events {
        // Mouse entered bounding box of node
        CreateEventDecl(touchable, mouseenter);
        // Mouse left bounding box of node
        CreateEventDecl(touchable, mouseleave);
        // LMB held down on node
        CreateEventDecl(touchable, mousedown);
        // LMB released from node
        CreateEventDecl(touchable, mouseup);
        // LMB pressed on node
        CreateEventDecl(touchable, click);
        // RMB pressed on node
        CreateEventDecl(touchable, context);
    };

    virtual bool init() override;

    static std::shared_ptr<Touchable> create();

    virtual void update(const double dt) override;

protected:
    Touchable();

    bool m_isHovered;
    std::shared_ptr<Engine::MouseData> m_lastMouseData;
};