#pragma once

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_blendmode.h>

#include "Node.h"

class ColorNode : public Node {
public:
    ColorNode();

    static std::shared_ptr<ColorNode> create();

    void setOpacity(unsigned char opacity);
    int8_t getOpacity();

    struct Color3 {
        unsigned char r;
        unsigned char g;
        unsigned char b;

        operator SDL_Color();

        operator SDL_FColor();
    };

    struct Color4 {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;

        operator SDL_Color();

        operator SDL_FColor();
    };

    enum BlendMode {
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

    Color3 getColor();
    // Includes alpha channel
    Color4 getColorA();

    void setBlendMode(BlendMode mode);
    BlendMode getBlendMode();

protected:
    Color4 m_color;
    BlendMode m_blendMode;
};