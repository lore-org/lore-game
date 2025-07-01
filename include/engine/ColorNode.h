#pragma once

#include <SDL3/SDL_pixels.h>

#include "Node.h"
#include "SDL3/SDL_pixels.h"

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

    void setColor(Color3 color);
    // Includes alpha channel
    void setColorA(Color4 color);

    Color3 getColor();
    // Includes alpha channel
    Color4 getColorA();

protected:
    Color4 m_color;
};