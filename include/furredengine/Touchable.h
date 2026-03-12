#pragma once

#include <memory>

#include <furredengine/Node.h>
#include <furredengine/Engine.h>
#include <furredengine/utils.h>

namespace FurredEngine {

// sets up events for interacting with Mouse input. Use `Engine::sharedInstance()->getMouseData()` to get the current data involving the mouse instead.
class Touchable : public Node {
public:
    struct Events {
        // Mouse entered bounding box of node
        CreateEventDecl(touchable, mouseenter);
        // Mouse left bounding box of node
        CreateEventDecl(touchable, mouseleave);

        // Mouse button held down on node
        CreateEventDecl(touchable, mousedown);
        // Mouse button released from node
        CreateEventDecl(touchable, mouseup);

        // LMB pressed on node
        CreateEventDecl(touchable, click);
        // MMB pressed on node
        CreateEventDecl(touchable, pick);
        // RMB pressed on node
        CreateEventDecl(touchable, context);
        // Forwards Mouse Button pressed on node
        CreateEventDecl(touchable, forward);
        // Backwards Mouse Button pressed on node
        CreateEventDecl(touchable, backward);
    };

    virtual bool init() override;

    static std::shared_ptr<Touchable> create();

    virtual void update(const long double dt) override;

    inline bool isHovered() { return m_isHovered; }
    inline bool isPressed() { return m_isPressed; }
    inline bool isFocused() { return m_isFocused; }

protected:
    Touchable();

    bool m_isHovered;
    bool m_isPressed;
    bool m_isFocused;
    
    Engine::MouseData m_lastMouseData;
};

}