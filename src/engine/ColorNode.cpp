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

std::shared_ptr<ColorNode> ColorNode::create() {
    auto ret = utils::protected_make_shared<ColorNode>();

    if (!ret->init()) return nullptr;
    return ret;
}

void ColorNode::setOpacity(unsigned char opacity) {
    m_color.a = opacity;
}

void ColorNode::setColor(Color3 color) {
    m_color.r = color.r;
    m_color.g = color.g;
    m_color.b = color.b;
}

void ColorNode::setColorA(Color4 color) {
    m_color = color;
}

void ColorNode::setBlendMode(BlendMode mode) {
    m_blendMode = mode;
}