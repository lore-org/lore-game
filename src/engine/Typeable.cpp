#include <furredengine/Typeable.h>

#include <cmath>
#include <memory>
#include <cstdint>

#include <furredengine/ColorNode.h>
#include <furredengine/RectangleNode.h>
#include <furredengine/TextNode.h>
#include <furredengine/Touchable.h>

using namespace FurredEngine;

using Color4 = ColorNode::Color4;

Typeable::Typeable() :
    m_inputType(InputType::Text), m_seekBounds({ 0, 0 }),
    m_widthToCursor(0) {}

bool Typeable::init(Color4 displayTextColor, Color4 placeholderTextColor) {
    if (!Touchable::init()) return false;

    auto rect = this->getRect();

    // ---- Display Text ----

    m_displayText = TextNode::create(
        nullptr,
        {
            rect.getMinX() + 2,
            rect.getMinY() + (rect.getHeight() / 2.)
        }
    );
    m_displayText->setAnchorPoint(0, 0.5);
    m_displayText->setColorA(displayTextColor);

    // ---- Placeholder Text ----

    m_placeholderText = TextNode::create(
        nullptr,
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
    m_cursor->setOpacity(m_displayText->getOpacity() / 2.f);

    // --------------------

    this->addChild(m_displayText, 1);
    this->addChild(m_placeholderText, 1);
    this->addChild(m_cursor, 2);

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
        { 255, 255, 255, 255 },
        { 255, 255, 255, 155 }
    )) return nullptr;
    return ret;
}

std::shared_ptr<Typeable> Typeable::createWithColors(Color4 displayTextColor, Color4 placeholderTextColor) {
    auto ret = utils::protected_make_shared<Typeable>();

    if (!ret->init(displayTextColor, placeholderTextColor)) return nullptr;
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

    auto& inputText = m_displayText->m_displayedText;

    // Placeholder Swapping
    m_placeholderText->setVisible(inputText.empty());

    // Cursor Blinking
    m_cursor->setVisible(m_isFocused && std::fmod(Engine::getTime(), .7) > .35);

    Touchable::draw(dt);
}

void Typeable::setScale(long double scale) {
    Touchable::setScale(scale);
    this->_updateChildren();
}

void Typeable::setPosition(long double x, long double y) {
    Touchable::setPosition(x, y);
    this->_updateChildren();
}

void Typeable::setAnchorPoint(long double x, long double y) {
    Touchable::setAnchorPoint(x, y);
    this->_updateChildren();
}

void Typeable::setContentSize(long double width, long double height) {
    Touchable::setContentSize(width, height);
    this->_updateChildren();
}

void Typeable::_focusIn(void* data) {
    auto engine = Engine::sharedInstance();
    auto& inputText = m_displayText->m_displayedText;

    auto window = engine->getWindow();
    engine->requestTextInputCapturing(utils::cast_shared<Typeable>(this));

    m_seekBounds.start = inputText.size();
}

void Typeable::_focusOut(void* data) {
    auto engine = Engine::sharedInstance();

    auto window = engine->getWindow();
    engine->removeTextInputCapturing(utils::cast_shared<Typeable>(this));
}

void Typeable::_handleText(std::string text) {
    if (m_seekBounds.length > 0) this->_handleDelete(DeleteType::Backwards);

    auto& inputText = m_displayText->m_displayedText;
    
    inputText.insert(m_seekBounds.start, text);
    m_seekBounds.start += text.size();

    this->_callEventListener(Events::changed, &text);
    
    m_displayText->m_statusBitset |= TextNode::UPDATE_VERTICES;

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

    m_displayText->m_statusBitset |= TextNode::UPDATE_VERTICES;

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, inputText.size()));
}

void Typeable::_handleSeeking(int32_t start, int32_t length) {
    SeekBounds seekBounds { start, length };
    auto& inputText = m_displayText->m_displayedText;

    m_seekBounds = seekBounds;
    this->_callEventListener(Events::seeked, &seekBounds);
    
    this->_measureString();

    LogInfo(fmt::format("start: {}; size: {}", m_seekBounds.start, inputText.size()));
}

void Typeable::_measureString() {
    auto& inputText = m_displayText->m_displayedText;

    auto glyphs = m_displayText->m_fontFace->loadString(m_displayText->m_displayedText);
    m_widthToCursor = 0;
    for (size_t i = 0; i < glyphs.size(); i++) {
        auto& glyph = glyphs[i];
        
        if (i < glyphs.size() - 1) m_widthToCursor += glyph->advanceX;
        else m_widthToCursor += glyph->offsetX + glyph->advanceX;
    }

    m_cursor->setPositionX(this->getRect().getMinX() + 2 + m_widthToCursor);
}

void Typeable::_updateChildren() {
    auto rect = this->getRect();

    // ---- Display Text ----

    m_displayText->setPosition(
        rect.getMinX() + 2,
        rect.getMinY() + (rect.getHeight() / 2.)
    );
    
    // ---- Placeholder Text ----

    m_placeholderText->setPosition(m_displayText->getPosition());
    
    // ---- Cursor ----

    m_cursor->setPosition(
        rect.getMinX() + 2 + m_widthToCursor,
        rect.getMinY() + 3
    );
    m_cursor->setContentHeight(rect.getHeight() - 6);
}