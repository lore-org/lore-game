#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include <string>

#include "Geometry.hpp"
#include "Node.hpp"
#include "utils.hpp"

class Sprite : public ColorNode {
public:
    virtual void release() override {
        if (--m_refCount <= 0) {
            UnloadTexture(m_texture);
            delete this;
        }
    };

    virtual bool init(Texture2D texture) {
        if (!ColorNode::init()) return false;

        this->setContentSize(Size(texture.width, texture.height));
        m_texture = texture;
        return true;
    }

    static Sprite* createFromFile(const std::string file) {
        auto ret = new Sprite();
        if (!ret->init(LoadTexture(file.c_str()))) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    };
    static Sprite* createWithImage(const Image image) {
        auto ret = new Sprite();
        if (!ret->init(LoadTextureFromImage(image))) {
            ret->release();
            UnloadImage(image);
            return nullptr;
        }
        
        UnloadImage(image);
        return ret;
    };
    static Sprite* createWithTexture(const Texture texture) {
        auto ret = new Sprite();
        if (!ret->init(texture)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
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

    virtual void draw(const double dt) override {
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
            m_color
        );

        ColorNode::draw(dt);
    }

protected:
    Texture m_texture;
};