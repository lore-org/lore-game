#pragma once

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <engine/ColorNode.h>
#include <engine/FontManager.h>

class TextNode : public ColorNode {
    friend class Typeable;
    friend class Engine;

public:
    virtual ~TextNode();

    virtual bool init(FontManager::FontFace* font, Point position);

    static std::shared_ptr<TextNode> create(FontManager::FontFace* font = nullptr, Point position = { 0, 0 });
    static std::shared_ptr<TextNode> create(std::string fontFile, Point position = { 0, 0 });

    virtual void draw(const long double dt) override;

    void setDisplayedText(std::string displayedText);
    inline std::string getDisplayedText() { return m_displayedText; }

    void setFontPoint(float point);
    inline float getFontPoint() { return m_fontFace->getFontPoint(); }

    // TODO - impl changeFontHeight, which changes the font size to the desired height, calculates the difference from the actual height, and adjusts for the pt ratio
    void changeFont(std::string fontFile);
    void changeFont(FontManager::FontFace* font);


    // overrides for statusBitset

    virtual void setScale(long double scale) override;

    virtual void setRotation(long double rotation) override;
    
    virtual void setPosition(long double x, long double y) override;
    virtual inline void setPosition(Point position) override { setPosition(position.x, position.y); }

    virtual void setAnchorPoint(long double x, long double y) override;
    virtual inline void setAnchorPoint(Point anchorPoint) override { setAnchorPoint(anchorPoint.x, anchorPoint.y); }

    virtual void setContentSize(long double width, long double height) override;
    virtual inline void setContentSize(Size contentSize) override { setContentSize(contentSize.width, contentSize.height); }

    virtual void setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override; // Includes alpha channel
    virtual inline void setColorA(Color4 color) override { setColorA(color.r, color.g, color.b, color.a); }; // Includes alpha channel

protected:
    TextNode();

    std::string m_displayedText;

    FontManager::FontFace* m_fontFace;


    struct BufferData {
        vec2<int> vertPos;
        vec2<unsigned int> texCoord;
    };

    GLuint m_glProgram;

    GLuint m_glVertexArray;
    GLuint m_glVertexBuffer;

    GLuint m_glTexture;
    

    size_t m_numChars;

private:

    enum : char {
        UPDATE_VERTICES =   1 << 0,
        UPDATE_COLOR =      1 << 1,
        UPDATE_ROTATION =   1 << 2
    };
    char m_statusBitset = 0b111;

    void _measureString();

    void _createAtlasTex();
    void _updateVertices();
};