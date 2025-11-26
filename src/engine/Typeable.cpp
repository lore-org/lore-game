#include "SDL3/SDL_timer.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "engine/ColorNode.h"
#include "engine/Touchable.h"
#include <cstdint>
#include <engine/Typeable.h>

#include <memory>

#include <SDL3/SDL.h>

Typeable::Typeable() : m_inputText(""), m_placeholderText("") {}

bool Typeable::init() {
    if (!Touchable::init()) return false;

    this->registerEventListener(
        Typeable::Events::focusin,
        std::make_shared<Event_Callback>(
            [this](auto data) { this->_focusIn(data); }
        )
    );

    this->registerEventListener(
        Typeable::Events::focusout,
        std::make_shared<Event_Callback>(
            [this](auto data) { this->_focusOut(data); }
        )
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

void Typeable::draw(const long double dt) {
    Touchable::draw(dt);
    
    auto engine = Engine::sharedInstance();

    // ---- Displayed Text ----

    // TODO - create Text node and add as child

    // TODO - add as member of class
    auto notoSans = engine->getOrCreateFont("resources/Noto Sans.ttf");
    // TODO - add as member of class
    auto displayText = TTF_CreateText(
        engine->getTextEngine(), notoSans,
        "", 0
    );

    auto rect = this->getRect();

    if (!TTF_SetFontSize(notoSans, rect.getHeight() - 8.)) LogSDLError();

    if (!TTF_SetTextString(
        displayText,
        m_inputText.c_str(), 0
    )) LogSDLError();
    
    if (!TTF_SetTextColor(
        displayText,
        0, 0, 0, 255
    )) LogSDLError();

    if (!TTF_DrawRendererText(
        displayText,
        rect.getMinX() + 2., rect.getMinY() + 2.
    )) LogSDLError();
    
    // ------------------------

    // ---- Cursor ----

    auto renderer = engine->getRenderer();

    if (!SDL_SetRenderDrawColor(
        renderer,
        0,
        0,
        0,
        m_isFocused ? ((SDL_GetTicks() % 700) > 350 ? 155 : 0) : 0 // blink every 500ms
    )) LogSDLError();
    if (!SDL_SetRenderDrawBlendMode(
        renderer,
        static_cast<SDL_BlendMode>(ColorNode::BlendMode::Blend)
    )) LogSDLError();

    int measuredWidth;
    size_t measuredLength;

    TTF_MeasureString(
        notoSans, m_inputText.substr(0, m_seekBounds.start).c_str(),
        0, 0,
        &measuredWidth, &measuredLength
    );

    SDL_FRect sdlRect = Rect {
        rect.getMinX() + static_cast<float>(measuredWidth) + 2., rect.getMinY() + 3.,
        1, rect.getHeight() - 6.
    };

    if (!SDL_RenderFillRect(
        renderer,
        &sdlRect
    )) LogSDLError();

    // ----------------
}

void Typeable::_focusIn(void* data) {
    auto engine = Engine::sharedInstance();

    auto window = engine->getWindow();
    engine->requestTextInputCapturing(utils::cast_shared<Typeable>(this));

    m_seekBounds.start = m_inputText.size();

    SDL_Rect rect = this->getRect();
    SDL_SetTextInputArea(window, &rect, 0);

    if (SDL_TextInputActive(window)) return;
    if (!SDL_StartTextInputWithProperties(
        window, static_cast<SDL_PropertiesID>(m_inputType)
    )) LogSDLError();
}

void Typeable::_focusOut(void* data) {
    auto engine = Engine::sharedInstance();

    auto window = engine->getWindow();
    engine->removeTextInputCapturing(utils::cast_shared<Typeable>(this));

    SDL_SetTextInputArea(window, nullptr, 0);

    if (!SDL_TextInputActive(window)) return;
    if (!SDL_StopTextInput(window)) LogSDLError();
}

void Typeable::_handleText(std::string text) {
    if (m_seekBounds.length > 0) this->_handleDelete(DeleteType::Backwards);
    
    m_inputText.insert(m_seekBounds.start, text);
    m_seekBounds.start += text.size();

    this->_callEventListener(Events::changed, &text);

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, m_inputText.size()));
}

void Typeable::_handleDelete(DeleteType type) {
    if (m_inputText.empty()) return;

    if (m_seekBounds.length <= 0) {
        switch (type) {
            case DeleteType::Backwards:
                if (m_inputText.empty()) break;
                m_inputText.erase(--m_seekBounds.start, 1);
                break;
            case DeleteType::Forwards:
                if (m_seekBounds.start >= m_inputText.size()) break;
                m_inputText.erase(m_seekBounds.start, 1);
                break;
        }
    } else {
        m_inputText.erase(m_seekBounds.start, m_seekBounds.length);
    }

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, m_inputText.size()));
}

void Typeable::_handleSeeking(int32_t start, int32_t length) {
    SeekBounds seekBounds { start, length };

    m_seekBounds = seekBounds;
    this->_callEventListener(Events::seeked, &seekBounds);

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, m_inputText.size()));
}