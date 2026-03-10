#pragma once

#include <furredengine/Node.h>

namespace FurredEngine {

class ColorNode : public Node {
public:
    ColorNode();

    static std::shared_ptr<ColorNode> create();

    struct Color4;

    struct Color3 {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        inline bool operator==(Color3& right) { return r == right.r && g == right.g && b == right.b; }
        inline operator Color4() { return { r, g, b, 255 }; }
    };

    struct Color4 {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        inline bool operator==(Color4& right) { return r == right.r && g == right.g && b == right.b && a == right.a; }
    };

    enum class BlendMode : char {
        None,
        Blend,
        Blend_Premultiplied,
        Add,
        Add_Premultiplied,
        Modulate,
        Multiply
    };
    
    
    virtual void setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Includes alpha channel
    virtual inline void setColorA(Color4 color) { setColorA(color.r, color.g, color.b, color.a); }; // Includes alpha channel
    
    inline Color4 getColorA() { return m_color; } // Includes alpha channel

    
    inline void setColor(uint8_t r, uint8_t g, uint8_t b) { setColorA(r, g, b, getOpacity()); };
    inline void setColor(Color3 color) { setColor(color.r, color.g, color.b); };

    inline Color3 getColor() { return { m_color.r, m_color.g, m_color.b }; }


    inline void setOpacity(uint8_t opacity) { setColorA(getColorA().r, getColorA().g, getColorA().b, opacity); };

    inline uint8_t getOpacity() { return m_color.a; }


    void setBlendMode(BlendMode mode);
    inline BlendMode getBlendMode() { return m_blendMode; }

protected:
    Color4 m_color;
    BlendMode m_blendMode;
};

}