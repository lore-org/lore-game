#pragma once

#include <string>

#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <Cimg/CImg.h>

#include "Node.hpp"
#include "raylib.h"

class Sprite : public Node {
public:
    Sprite(Texture2D texture) : m_texture(texture) {};
    virtual ~Sprite() {
        UnloadTexture(m_texture);
    }

    static Sprite* createFromFile(const std::string file) {
        return new Sprite(LoadTexture(file.c_str()));
    };
    static Sprite* createWithImage(const Image image) {
        auto spr = new Sprite(LoadTextureFromImage(image));
        UnloadImage(image);
        return spr;
    };
    static Sprite* createWithTexture(const Texture texture) {
        return new Sprite(texture);
    };

    inline operator Image() const {
        return LoadImageFromTexture(m_texture);
    }

    inline operator Texture() const {
        return m_texture;
    }

    inline void setImage(const Image image) {
        m_texture = LoadTextureFromImage(image);
    };
    inline void setTexture(const Texture texture) {
        m_texture = texture;
    }

    virtual void draw() const {
        auto xOffset = m_anchorPoint.x == 0 ? 0 : m_texture.width * m_anchorPoint.x;
        auto yOffset = m_anchorPoint.y == 0 ? 0 : m_texture.height * m_anchorPoint.y;
        DrawTextureEx(m_texture, m_position - Point(xOffset, yOffset), m_rotation, m_scale, WHITE);
    }

protected:
    Texture m_texture;
};