#include <engine/ColorNode.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

ColorNode::ColorNode() : m_color({ 255, 255, 255, 255 }), m_blendMode(Blend) {}

ColorNode::Color3::operator SDL_Color() {
    return {
        r, g, b, 255
    };
}

ColorNode::Color3::operator SDL_FColor() {
    return {
        static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), 255.f
    };
}

ColorNode::Color4::operator SDL_Color() {
    return {
        r, g, b, a
    };
}

ColorNode::Color4::operator SDL_FColor() {
    return {
        static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a)
    };
}

std::shared_ptr<ColorNode> ColorNode::create() {
    auto ret = utils::protected_make_shared<ColorNode>();

    if (!ret->init()) return nullptr;
    return ret;
}

void ColorNode::setOpacity(unsigned char opacity) {
    m_color.a = opacity;
}

int8_t ColorNode::getOpacity() {
    return m_color.a;
}

void ColorNode::setColor(Color3 color) {
    m_color.r = color.r;
    m_color.g = color.g;
    m_color.b = color.b;
}

void ColorNode::setColorA(Color4 color) {
    m_color = color;
}

ColorNode::Color3 ColorNode::getColor() {
    return { m_color.r, m_color.g, m_color.b };
}

ColorNode::Color4 ColorNode::getColorA() {
    return m_color;
}

void ColorNode::setBlendMode(BlendMode mode) {
    m_blendMode = mode;
}

ColorNode::BlendMode ColorNode::getBlendMode() {
    return m_blendMode;
}