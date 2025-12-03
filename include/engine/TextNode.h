#pragma once

#include <engine/ColorNode.h>

class TextNode : public ColorNode {
    friend class Typeable;
    friend class Engine;

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

    // TODO - impl changeFontHeight, which changes the font size to the desired height, calculates the difference from the actual height, and adjusts for the pt ratio

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

private:

    // TODO - impl TTF_SetTextWrapWidth, TTF_SetTextDirection

    void _updateTextString();
    void _updateTextColor();
    void _updateTextFont();
    // Also calls TTF_SetFontSize
    void _updateContentSize();


    // Replaces default font if needed, copies the font to its own buffer, and checks for any errors.
    void _copyAndVerifyFont(TTF_Font* font);
    void _measureString();
};