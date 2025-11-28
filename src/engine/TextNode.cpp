#include <engine/TextNode.h>

#include <SDL3_ttf/SDL_ttf.h>

#include <engine/utils.hpp>
#include <engine/Engine.h>

TextNode::TextNode() :
    m_displayedText(""), m_fontSize(14) {}

TextNode::~TextNode() {
    TTF_DestroyText(m_text);
}

bool TextNode::init(TTF_Font* font, float fontSize, Point position) {
    if (!ColorNode::init()) return false;

    auto engine = Engine::sharedInstance();

    if (font == nullptr) {
        font = engine->getOrCreateFont("resources/Noto Sans.ttf");
    }

    m_font = font;
    m_text = TTF_CreateText(
        engine->getTextEngine(), m_font,
        "", 0
    );
    m_position = position;

    this->_updateTextString();
    this->_updateTextColor();
    this->_updateTextFont();
    this->_updateContentSize();

    return true;
}

std::shared_ptr<TextNode> TextNode::create() {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        nullptr, 0,
        { 0, 0 }
    )) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::createWithFont(std::string fontFile, float fontSize) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        Engine::sharedInstance()->getOrCreateFont(fontFile), fontSize,
        { 0, 0 }
    )) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::createWithFont(TTF_Font* font, float fontSize) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        font, 0,
        { 0, 0 }
    )) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::createWithPosition(Point position) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        nullptr, 0,
        position
    )) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::createWithData(std::string fontFile, float fontSize, Point position) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        Engine::sharedInstance()->getOrCreateFont(fontFile), fontSize,
        position
    )) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::createWithData(TTF_Font* font, float fontSize, Point position) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (!ret->init(
        font, fontSize,
        position
    )) return nullptr;
    return ret;
}

void TextNode::draw(const long double dt) {
    ColorNode::draw(dt);

    if (!this->isVisible()) return;

    auto rect = this->getRect();
    
    this->_updateFontSize();

    if (!TTF_DrawRendererText(
        m_text,
        rect.getMinX(), rect.getMinY()
    )) LogSDLError();
}

void TextNode::setDisplayedText(std::string displayedText) {
    if (m_displayedText == displayedText) return;

    m_displayedText = displayedText;
    this->_updateTextString();
    this->_updateContentSize();
}

void TextNode::setFontSize(float fontSize) {
    if (m_fontSize == fontSize) return;

    m_fontSize = fontSize;
    this->_updateContentSize();
}

void TextNode::changeFont(std::string fontFile) {
    this->changeFont(Engine::sharedInstance()->getOrCreateFont(fontFile));
}

void TextNode::changeFont(TTF_Font* font) {
    if (m_font == font) return;

    m_font = font;
    this->_updateTextFont();
    this->_updateContentSize();
}

void TextNode::setOpacity(uint8_t opacity) {
    if (m_color.a == opacity) return;

    ColorNode::setOpacity(opacity);
    this->_updateTextColor();
}

void TextNode::setColor(Color3 color) {
    if (
        m_color.r == color.r &&
        m_color.g == color.g &&
        m_color.b == color.b
    ) return;

    ColorNode::setColor(color);
    this->_updateTextColor();
}

void TextNode::setColorA(Color4 color) {
    if (m_color == color) return;

    ColorNode::setColorA(color);
    this->_updateTextColor();
}

void TextNode::_updateTextString() {
    if (!TTF_SetTextString(
        m_text,
        m_displayedText.c_str(), m_displayedText.size()
    )) LogSDLError();
}

void TextNode::_updateTextColor() {
    if (!TTF_SetTextColor(
        m_text,
        m_color.r, m_color.g, m_color.b, m_color.a
    )) LogSDLError();
}

void TextNode::_updateTextFont() {
    if (!TTF_SetTextFont(
        m_text, m_font
    )) LogSDLError();
}

void TextNode::_updateContentSize() {
    this->_updateFontSize();

    int width;
    int height;
    
    if (!TTF_GetTextSize(
        m_text,
        &width, &height
    )) LogSDLError();

    this->setContentSize(width, height);
}

void TextNode::_updateFontSize() {
    if (!TTF_SetFontSize(
        m_font, m_fontSize
    )) LogSDLError();
}