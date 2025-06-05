#pragma once

#include <string>

#include <raylib-cpp.hpp>
#include <fmt/base.h>

#include "Data.hpp"
#include "Node.hpp"
#include "raylib.h"

class Sprite : public Node {
public:
    Sprite(Texture2D texture) : m_texture(texture) {
        this->setContentSize(Size(texture.width, texture.height));
    };

    virtual void release() {
        if (--m_refCount <= 0) {
            UnloadTexture(m_texture);
            delete this;
        }
    };

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
    inline Image getImage() const {
        return LoadImageFromTexture(m_texture);
    }
    inline Texture getTexture() const {
        return m_texture;
    }

    virtual void draw() const {
        auto xOffset = IsZero(m_anchorPoint.x) ? 0 : m_texture.width * m_anchorPoint.x;
        auto yOffset = IsZero(m_anchorPoint.y) ? 0 : m_texture.height * m_anchorPoint.y;
        DrawTexturePro(
            m_texture,
            Rect(
                Point(0),
                Size(m_texture.width, m_texture.height)
            ),
            Rect(
                m_position,
                m_contentSize
            ),
            { xOffset, yOffset },
            m_rotation,
            WHITE
        );

        std::for_each(
            m_children.begin(),
            m_children.end(),
            [](Node* child) { child->draw(); }
        );
    }

protected:
    Texture m_texture;
};