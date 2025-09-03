#include "SDL3/SDL_keyboard.h"
#include <engine/Typeable.h>
#include <memory>

Typeable::Typeable() : m_inputText(""), m_placeholderText("") {}

bool Typeable::init() {
    if (!Touchable::init()) return false;

    this->registerEventListener(
        Touchable::Events::focusin,
        std::make_shared<Event_Callback>(Typeable::_focusIn)
    );

    return true;
}

void Typeable::update(const long double dt) {
    Touchable::update(dt);
}

void Typeable::_focusIn(std::shared_ptr<void> data) {
    SDL_RunOnMainThread([](auto data) {
        auto window = static_cast<SDL_Window*>(data);
        
        if (!SDL_TextInputActive(window))
            SDL_StartTextInput(window); // TODO - impl Typeable::InputType
    }, Engine::sharedInstance()->getWindow(), false);
}

void Typeable::_focusOut(std::shared_ptr<void> data) {
    SDL_RunOnMainThread([](auto data) {
        auto window = static_cast<SDL_Window*>(data);
        
        if (SDL_TextInputActive(window))
            SDL_StopTextInput(window);
    }, Engine::sharedInstance()->getWindow(), false);
}