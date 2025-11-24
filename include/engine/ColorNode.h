#pragma once

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_blendmode.h>

#include <engine/Node.h>

class ColorNode : public Node {
public:
    ColorNode();

    static std::shared_ptr<ColorNode> create();

    void setOpacity(unsigned char opacity);
    inline int8_t getOpacity() { return m_color.a; }

    struct Color3 {
        unsigned char r;
        unsigned char g;
        unsigned char b;

        inline operator SDL_Color() { return { r, g, b, 255 }; }

        inline operator SDL_FColor() { return { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), 255.f }; }
    };

    struct Color4 {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;

        inline operator SDL_Color() { return { r, g, b, a }; }

        inline operator SDL_FColor() { return { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a) }; }
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