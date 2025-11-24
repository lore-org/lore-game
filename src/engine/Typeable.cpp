#include "SDL3/SDL_keyboard.h"
#include <engine/Typeable.h>
#include <memory>

Typeable::Typeable() : m_inputText(""), m_placeholderText("") {}

bool Typeable::init() {
    if (!Touchable::init()) return false;

    this->registerEventListener(
        Typeable::Events::focusin,
        std::make_shared<Event_Callback>(Typeable::_focusIn)
    );

    this->registerEventListener(
        Typeable::Events::focusout,
        std::make_shared<Event_Callback>(Typeable::_focusOut)
    );

    return true;
}

std::shared_ptr<Typeable> Typeable::create() {
    auto ret = utils::protected_make_shared<Typeable>();

    if (!ret->init()) return nullptr;
    return ret;
}

void Typeable::update(const long double dt) {
    auto wasHovered = m_isHovered;
    auto wasPressed = m_isPressed;
    auto wasFocused = m_isFocused;

    Touchable::update(dt);

    if (!wasFocused && m_isFocused) this->_callEventListener(Events::focusin);
    else if (!m_isFocused && wasFocused) this->_callEventListener(Events::focusout);
}

void Typeable::_focusIn(std::shared_ptr<void> data) {
    auto window = Engine::sharedInstance()->getWindow();

    // TODO - impl Typeable::InputType
    // TODO - add text capturing in main loop
    if (!SDL_TextInputActive(window)) SDL_StartTextInput(window);
}

void Typeable::_focusOut(std::shared_ptr<void> data) {
    auto window = Engine::sharedInstance()->getWindow();
        
    if (SDL_TextInputActive(window)) SDL_StopTextInput(window);
}