#pragma once

#include <SDL3/SDL.h>

#include <engine/Node.h>

class ColorNode : public Node {
public:
    ColorNode();

    static std::shared_ptr<ColorNode> create();

    void setOpacity(uint8_t opacity);
    inline uint8_t getOpacity() { return m_color.a; }

    struct Color4;

    struct Color3 {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        inline operator SDL_Color() { return { r, g, b, 255 }; }
        inline operator SDL_FColor() { return { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), 255.f }; }
        inline bool operator==(Color3& right) { return r == right.r && g == right.g && b == right.b; }
        inline operator Color4() { return { r, g, b, 255 }; }
    };

    struct Color4 {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        inline operator SDL_Color() { return { r, g, b, a }; }
        inline operator SDL_FColor() { return { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a) }; }
        inline bool operator==(Color4& right) { return r == right.r && g == right.g && b == right.b && a == right.a; }
    };

    enum class BlendMode : SDL_BlendMode {
        None = SDL_BLENDMODE_NONE,
        Blend = SDL_BLENDMODE_BLEND,
        Blend_Premultiplied = SDL_BLENDMODE_BLEND_PREMULTIPLIED,
        Add = SDL_BLENDMODE_ADD,
        Add_Premultiplied = SDL_BLENDMODE_ADD_PREMULTIPLIED,
        Modulate = SDL_BLENDMODE_MOD,
        Multiply = SDL_BLENDMODE_MUL
    };

    void setColor(Color3 color);
    // Includes alpha channel
    void setColorA(Color4 color);

    inline Color3 getColor() { return { m_color.r, m_color.g, m_color.b }; }
    // Includes alpha channel
    inline Color4 getColorA() { return m_color; }

    void setBlendMode(BlendMode mode);
    inline BlendMode getBlendMode() { return m_blendMode; }

protected:
    Color4 m_color;
    BlendMode m_blendMode;
};