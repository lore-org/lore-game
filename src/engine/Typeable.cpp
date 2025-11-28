#include <engine/Typeable.h>

#include <memory>
#include <cstdint>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <engine/ColorNode.h>
#include <engine/RectangleNode.h>
#include <engine/TextNode.h>
#include <engine/Touchable.h>

typedef ColorNode::Color4 Color4;

Typeable::Typeable() :
    m_inputType(InputType::Text), m_seekBounds({ 0, 0 }) {}

bool Typeable::init(Color4 displayTextColor, Color4 placeholderTextColor, Color4 backgroundColor) {
    if (!Touchable::init()) return false;

    auto rect = this->getRect();

    // ---- Display Text ----

    m_displayText = TextNode::createWithData(
        "resources/Noto Sans.ttf",
        rect.getHeight() - 8,
        { rect.getMinX() + 2, rect.getMinY() + 2 }
    );
    m_displayText->setAnchorPoint(0);
    m_displayText->setColorA(displayTextColor);

    // ---- Placeholder Text ----

    m_placeholderText = TextNode::createWithData(
        "resources/Noto Sans.ttf",
        m_displayText->getFontSize(),
        m_displayText->getPosition()
    );
    m_placeholderText->setAnchorPoint(m_displayText->getAnchorPoint());
    m_placeholderText->setColorA(placeholderTextColor);

    // ---- Cursor ----

    m_cursor = RectangleNode::createWithRect({
        rect.getMinX() + 2, rect.getMinY() + 3,
        2, rect.getHeight() - 6
    });
    m_cursor->setAnchorPoint(0);
    m_cursor->setColor(m_displayText->getColor());
    m_cursor->setOpacity(190);

    // ---- Background ----

    m_background = RectangleNode::createWithRect(rect);
    m_background->setAnchorPoint(0);
    m_background->setColorA(backgroundColor);

    // --------------------

    this->addChild(m_displayText, 1);
    this->addChild(m_placeholderText, 1);
    this->addChild(m_cursor, 2);
    this->addChild(m_background, 0);

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

    if (!ret->init(
        { 0, 0, 0, 255 },
        { 0, 0, 0, 155 },
        { 255, 255, 255, 255 }
    )) return nullptr;
    return ret;
}

std::shared_ptr<Typeable> Typeable::createWithColors(Color4 displayTextColor, Color4 placeholderTextColor, Color4 backgroundColor) {
    auto ret = utils::protected_make_shared<Typeable>();

    if (!ret->init(displayTextColor, placeholderTextColor, backgroundColor)) return nullptr;
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
    if (!this->isVisible()) {
        Touchable::draw(dt);
        return;
    }

    auto rect = this->getRect();
    auto& inputText = m_displayText->m_displayedText;

    m_displayText->_updateFontSize();
    
    int measuredWidth;
    size_t _measuredLength;

    TTF_MeasureString(
        m_displayText->m_font, inputText.substr(0, m_seekBounds.start).c_str(),
        0, 0,
        &measuredWidth, &_measuredLength
    );

    // ---- Display Text ----

    m_displayText->setPosition(
        rect.getMinX() + 2,
        rect.getMinY() + 2
    );
    m_displayText->setFontSize(rect.getHeight() - 8);
    
    // ---- Placeholder Text ----

    m_placeholderText->setPosition(m_displayText->getPosition());
    m_placeholderText->setFontSize(m_displayText->getFontSize());
    
    // ---- Cursor ----

    m_cursor->setPosition(
        rect.getMinX() + 2 + measuredWidth,
        rect.getMinY() + 3
    );
    m_cursor->setContentHeight(rect.getHeight() - 6);
    
    // ---- Background ----

    m_background->setPosition(rect.origin);
    m_background->setContentSize(rect.size);
    
    // --------------------

    // Placeholder Swapping
    m_placeholderText->setVisible(inputText.empty());

    // Cursor Blinking
    m_cursor->setVisible(m_isFocused && (SDL_GetTicks() % 700) > 350);

    Touchable::draw(dt);
}

void Typeable::_focusIn(void* data) {
    auto engine = Engine::sharedInstance();
    auto& inputText = m_displayText->m_displayedText;

    auto window = engine->getWindow();
    engine->requestTextInputCapturing(utils::cast_shared<Typeable>(this));

    m_seekBounds.start = inputText.size();

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

    auto& inputText = m_displayText->m_displayedText;
    
    inputText.insert(m_seekBounds.start, text);
    m_seekBounds.start += text.size();

    this->_callEventListener(Events::changed, &text);
    
    m_displayText->_updateTextString();

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, inputText.size()));
}

void Typeable::_handleDelete(DeleteType type) {
    auto& inputText = m_displayText->m_displayedText;
    if (inputText.empty()) return;

    if (m_seekBounds.length <= 0) {
        switch (type) {
            case DeleteType::Backwards:
                if (inputText.empty()) break;
                inputText.erase(--m_seekBounds.start, 1);
                break;
            case DeleteType::Forwards:
                if (m_seekBounds.start >= inputText.size()) break;
                inputText.erase(m_seekBounds.start, 1);
                break;
        }
    } else {
        inputText.erase(m_seekBounds.start, m_seekBounds.length);
    }

    m_displayText->_updateTextString();

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, inputText.size()));
}

void Typeable::_handleSeeking(int32_t start, int32_t length) {
    SeekBounds seekBounds { start, length };
    auto& inputText = m_displayText->m_displayedText;

    m_seekBounds = seekBounds;
    this->_callEventListener(Events::seeked, &seekBounds);

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, inputText.size()));
}