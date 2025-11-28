#pragma once

#include <engine/ColorNode.h>
#include <engine/Engine.h>

class Typeable;

class TextNode : public ColorNode {
public:
    virtual ~TextNode();

    virtual bool init(TTF_Font* font, float fontSize, Point position);

    static std::shared_ptr<TextNode> create();

    static std::shared_ptr<TextNode> createWithFont(std::string fontFile, float fontSize);
    static std::shared_ptr<TextNode> createWithFont(TTF_Font* font, float fontSize);
    
    static std::shared_ptr<TextNode> createWithPosition(Point position);

    static std::shared_ptr<TextNode> createWithData(std::string fontFile, float fontSize, Point position);
    static std::shared_ptr<TextNode> createWithData(TTF_Font* font, float fontSize, Point position);

    virtual void draw(const long double dt) override;

    void setDisplayedText(std::string displayedText);
    inline std::string getDisplayedText() { return m_displayedText; }

    void setFontSize(float fontSize);
    inline float getFontSize() { return m_fontSize; }

    inline TTF_Font* getSDLFont() { return m_font; }
    inline TTF_Text* getSDLText() { return m_text; }

    void changeFont(std::string fontFile);
    void changeFont(TTF_Font* font);

    // -- overrides from ColorNode --

    void setOpacity(uint8_t opacity);
    void setColor(Color3 color);
    // Includes alpha channel
    void setColorA(Color4 color);

    // -------------------------

protected:
    TextNode();

    std::string m_displayedText;

    float m_fontSize;

    TTF_Font* m_font;
    TTF_Text* m_text;

    friend Typeable;
    friend void Engine::runEngine();

private:

    // TODO - impl TTF_SetTextWrapWidth, TTF_SetTextDirection

    void _updateTextString();
    void _updateTextColor();
    void _updateTextFont();
    void _updateContentSize();
    void _updateFontSize();
};