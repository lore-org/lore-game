#include <engine/Button.h>

#include <memory>

#include <engine/ColorNode.h>
#include <engine/Touchable.h>

template <> struct fmt::formatter<Button::TouchState> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(Button::TouchState level, FormatContext& ctx) const {
        std::string_view name;
        switch (level) {
            case Button::TouchState::Base: name = "Base"; break;
            case Button::TouchState::Hovered: name = "Hovered"; break;
            case Button::TouchState::Pressed: name = "Pressed"; break;
        }
        return fmt::formatter<std::string_view>::format(name, ctx);
    }
};

// TODO - actually figure out how to use this properly

Button::Button() : m_currentState(Button::TouchState::Base), m_hovered(false), m_pressed(false) {};

bool Button::init() {
    if (!Touchable::init()) return false;
    if (!ColorNode::init()) return false;

    Touchable::registerEventListener(Touchable::Events::mouseenter, std::make_shared<Event_Callback>([this](auto){
        m_hovered = true;
    }));

    Touchable::registerEventListener(Touchable::Events::mouseleave, std::make_shared<Event_Callback>([this](auto){
        m_hovered = false;
    }));

    Touchable::registerEventListener(Touchable::Events::mousedown, std::make_shared<Event_Callback>([this](auto){
        m_pressed = true;
    }));

    Touchable::registerEventListener(Touchable::Events::mouseup, std::make_shared<Event_Callback>([this](auto){
        m_pressed = false;
    }));

    Touchable::registerEventListener(Touchable::Events::click, std::make_shared<Event_Callback>([this](auto){
        this->onClick();
    }));

    Touchable::registerEventListener(Touchable::Events::context, std::make_shared<Event_Callback>([this](auto){
        this->onContext();
    }));

    return true;
}

std::shared_ptr<Button> Button::create() {
    auto ret = utils::protected_make_shared<Button>();

    if (!ret->init()) return nullptr;
    return ret;
}

void Button::update(const long double dt) {
    Touchable::update(dt);

    if (m_hovered && !m_pressed) m_currentState = TouchState::Hovered;
    else if (m_pressed) m_currentState = TouchState::Pressed;
    else m_currentState = TouchState::Base;

    LogInfo(m_currentState);

    ColorNode::update(dt);
}

void Button::onPointerEnter() {}

void Button::onPointerLeave() {}

void Button::onPress() {}

void Button::onRelease() {}

void Button::onClick() {}

void Button::onContext() {}