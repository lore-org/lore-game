#include <fmt/format.h>
#include <memory>

#include <engine/ColorNode.h>
#include <engine/Touchable.h>

class Button : public Touchable, public ColorNode {
public:

    enum class TouchState {
        Base,
        Hovered,
        Pressed
    };

    virtual bool init() override;

    static std::shared_ptr<Button> create();

    virtual void update(const long double dt) override;

    // Called when `Touchable::Events::mouseenter` is recieved
    virtual void onPointerEnter();
    // Called when `Touchable::Events::mouseleave` is recieved
    virtual void onPointerLeave();
    // Called when `Touchable::Events::mousedown` is recieved
    virtual void onPress();
    // Called when `Touchable::Events::mouseup` is recieved
    virtual void onRelease();
    // Called when `Touchable::Events::click` is recieved
    virtual void onClick();
    // Called when `Touchable::Events::context` is recieved
    virtual void onContext();

protected:
    Button();

    TouchState m_currentState;
    bool m_hovered;
    bool m_pressed;
};