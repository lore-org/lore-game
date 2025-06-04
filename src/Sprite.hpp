#pragma once

#include <string>

#include <raylib-cpp.hpp>

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
        return new Sprite(LoadTextureFromImage(image));
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
        DrawTextureEx(m_texture, m_position, m_rotation, m_scale, {1, 1, 1});
    }

protected:
    Texture m_texture;
};