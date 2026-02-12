#include <engine/ColorNode.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

ColorNode::ColorNode() : m_color({ 255, 255, 255, 255 }), m_blendMode(BlendMode::Blend) {}

std::shared_ptr<ColorNode> ColorNode::create() {
    auto ret = utils::protected_make_shared<ColorNode>();

    if (!ret->init()) return nullptr;
    return ret;
}

void ColorNode::setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    m_color.r = r;
    m_color.g = g;
    m_color.b = b;
    m_color.a = a;
}

void ColorNode::setBlendMode(BlendMode mode) {
    m_blendMode = mode;
}