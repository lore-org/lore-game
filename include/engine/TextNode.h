#pragma once

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <Trex/Atlas.hpp>
#include "Trex/TextShaper.hpp"

#include <engine/ColorNode.h>

#ifndef DEFAULT_FONT_POINT
#define DEFAULT_FONT_POINT 28
#endif

class TextNode : public ColorNode {
    friend class Typeable;
    friend class Engine;

public:
    virtual ~TextNode();

    virtual bool init(Trex::Atlas* font, float fontPoint, Point position);

    static std::shared_ptr<TextNode> create(Trex::Atlas* font = nullptr, float fontPoint = DEFAULT_FONT_POINT, Point position = { 0, 0 });
    static std::shared_ptr<TextNode> create(std::string fontFile, float fontPoint = DEFAULT_FONT_POINT, Point position = { 0, 0 });

    virtual void draw(const long double dt) override;

    void setDisplayedText(std::string displayedText);
    inline std::string getDisplayedText() { return m_displayedText; }

    void setFontPoint(float fontPoint);
    inline float getFontPoint() { return m_fontPoint; }

    // TODO - impl changeFontHeight, which changes the font size to the desired height, calculates the difference from the actual height, and adjusts for the pt ratio
    void changeFontAtlas(std::string fontFile);
    void changeFontAtlas(Trex::Atlas* font);


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
    float m_fontPoint;

    Trex::Atlas* m_fontAtlas;
    Trex::TextShaper* m_fontTextShaper;


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
        UPDATE_ROTATION =   1 << 2,
        UPDATE_ATLAS =      1 << 3
    };
    char m_statusBitset = 0b1111;

    void _measureString();

    void _createAtlasTex();
    void _updateVertices();
};