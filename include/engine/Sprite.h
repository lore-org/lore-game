#pragma once

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

#include <memory>

#include <engine/ColorNode.h>
#include <winsock2.h>

class Sprite : public ColorNode {
public:
    virtual ~Sprite();

    struct Texture {
        unsigned char* data;
        int width;
        int height;
        enum Channels : int {
            Grayscale = 1,
            GrayscaleAlpha = 2,
            RGB = 3,
            RGBA = 4
        } channels;
    };

    virtual bool init(Texture* texture);

    static std::shared_ptr<Sprite> create();
    static std::shared_ptr<Sprite> createFromFile(std::string filename);
    // Uses a web URL to load an image
    static std::shared_ptr<Sprite> createFromURL(std::string url);
    // Copies the sprite's texture to a new instance
    static std::shared_ptr<Sprite> createFromSprite(std::shared_ptr<Sprite> sprite);

    void setTexture(Texture* texture);
    inline Texture* getTexture() { return m_texture; }

    virtual void draw(const long double dt) override;

    static Texture* loadFromURL(std::string url);


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
    Sprite();

    Texture* m_texture;
    
    
    struct BufferData {
        vec2<int> vertPos;
        vec2<unsigned int> texCoord;
    };

    GLuint m_glProgram;

    GLuint m_glVertexArray;
    GLuint m_glVertexBuffer;

    GLuint m_glTexture;

private:
    // TODO - apply this philosophy to other classes
    using ColorNode::init;


    enum : char {
        UPDATE_VERTICES =   1 << 0,
        UPDATE_COLOR =      1 << 1,
        UPDATE_ROTATION =   1 << 2,
        UPDATE_TEXTURE =    1 << 3
    };
    char m_statusBitset = 0b1111;

    void _createTexture();
    void _updateVertices();
};