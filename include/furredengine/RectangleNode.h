#pragma once

#include <memory>

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <furredengine/ColorNode.h>

namespace FurredEngine {

class RectangleNode : public ColorNode {
public:
    virtual ~RectangleNode();

    virtual bool init(Point origin, Size size);

    static std::shared_ptr<RectangleNode> create();

    static std::shared_ptr<RectangleNode> createWithVec(Point origin, Size size);

    static std::shared_ptr<RectangleNode> createWithRect(Rect rectangle);

    virtual void draw(const long double dt) override;


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
    RectangleNode();

    // unused
    bool m_filled;

    struct BufferData {
        vec2<int> vertPos;
    };

    GLuint m_glVertexArray;
    GLuint m_glVertexBuffer;

    GLuint m_glProgram;

private:
    using ColorNode::init;

    enum : char {
        UPDATE_VERTICES =   1 << 0,
        UPDATE_COLOR =      1 << 1,
        UPDATE_ROTATION =   1 << 2
    };
    char m_statusBitset = 0b111;

    void _updateVertices();
};

}