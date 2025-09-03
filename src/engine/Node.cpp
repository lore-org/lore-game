#include <engine/Node.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

#include <algorithm>
#include <memory>
#include <vector>

#include <engine/Object.h>
#include <engine/Geometry.h>
#include <engine/Scheduler.h>
#include <engine/Engine.h>

Node::Node() :
    m_rotation(.0f), m_scale(1.f), m_position(0),
    m_anchorPoint(.5f), m_contentSize(0),
    m_zOrder(0),
    m_parent(nullptr), m_tag(0), m_userData(nullptr), m_visible(true) {}

Node::~Node() {    
    this->cleanup();
}

std::shared_ptr<Node> Node::create() {
    auto ret = utils::protected_make_shared<Node>();

    if (!ret->init()) return nullptr;
    return ret;
}

void Node::setZOrder(int64_t zOrder) {
    m_zOrder = zOrder;
    if (m_parent) m_parent->sortAllChildren();
}

void Node::setScale(long double scale) {
    m_scale = scale;
}

void Node::setPosition(Point position) {
    m_position = position;
}

void Node::setPosition(long double x, long double y) {
    m_position.x = x;
    m_position.y = y;
}

void Node::setPositionX(long double x) {
    m_position.x = x;
}

void Node::setPositionY(long double y) {
    m_position.y = y;
}

void Node::setAnchorPoint(Point anchorPoint) {
    m_anchorPoint = anchorPoint;
}

void Node::setAnchorPoint(long double x, long double y) {
    m_anchorPoint.x = x;
    m_anchorPoint.y = y;
}

void Node::setAnchorX(long double x) {
    m_anchorPoint.x = x;
}

void Node::setAnchorY(long double y) {
    m_anchorPoint.y = y;
}

// TODO - implement NULL for filling render target with contentSize

void Node::setContentSize(Size contentSize) {
    m_contentSize = contentSize;
}

void Node::setContentSize(long double width, long double height) {
    m_contentSize.width = width;
    m_contentSize.height = height;
}

void Node::setContentWidth(long double width) {
    m_contentSize.width = width;
}

void Node::setContentHeight(long double height) {
    m_contentSize.height = height;
}

void Node::setVisible(bool visible) {
    m_visible = visible;
}

void Node::setRotation(long double rotation) {
    m_rotation = rotation;
}

// FIXME - point is always inside rect?
bool Node::containsPoint(Point point) {
    auto scaledWidth = m_contentSize.width * m_anchorPoint.x;
    auto scaledHeight = m_contentSize.height * m_anchorPoint.y;

    auto leftBound = m_position.x - scaledWidth;
    auto rightBound = m_position.x + (m_contentSize.width - scaledWidth);
    auto topBound = m_position.y - scaledHeight;
    auto bottomBound = m_position.y + (m_contentSize.height - scaledHeight);

    auto center = MakePoint(m_position.x + scaledWidth, m_position.y + scaledHeight);

    /**
     * A----------B
     * |          |
     * |          |
     * D----------C
     */

    auto A = MakePoint(leftBound, topBound).rotateAroundCenter(center, m_rotation);
    auto B = MakePoint(rightBound, topBound).rotateAroundCenter(center, m_rotation);
    auto C = MakePoint(rightBound, bottomBound).rotateAroundCenter(center, m_rotation);
    auto D = MakePoint(leftBound, bottomBound).rotateAroundCenter(center, m_rotation);

    #define Vector(p1, p2) MakePoint(p2.x - p1.x, p2.y - p1.y)

    auto AB = Vector(A, B);
    auto AM = Vector(A, point);
    auto BC = Vector(B, C);
    auto BM = Vector(B, point);

    return
        0 <= utils::dot(AB, AM) &&
        utils::dot(AB, AM) <= utils::dot(AB, AB) &&
        0 <= utils::dot(BC, BM) &&
        utils::dot(BC, BM) <= utils::dot(BC, BC);
}

void Node::addChild(std::shared_ptr<Node> child) {
    child->m_parent = utils::cast_shared<Node>(this);
    m_children.push_back(child);
    this->sortAllChildren();
}

void Node::addChild(std::shared_ptr<Node> child, int64_t zOrder) {
    child->setZOrder(zOrder);
    this->addChild(child);
}

void Node::addChild(std::shared_ptr<Node> child, int64_t zOrder, int64_t tag) {
    this->setTag(tag);
    this->addChild(child, zOrder);
}

std::shared_ptr<Node> Node::getChildByTag(int64_t tag) {
    auto find = std::ranges::find_if(
        m_children,
        [tag](std::shared_ptr<Node> node) { return node->m_tag == tag; }
    );

    if (find != m_children.end()) return *find;
    return nullptr;
}

void Node::setParent(std::shared_ptr<Node> parent) {
    if (m_parent) m_parent->removeChild(utils::cast_shared<Node>(this));
    parent->addChild(utils::cast_shared<Node>(this));
    m_parent = parent;
}

void Node::removeFromParent() {
    m_parent->removeChild(utils::cast_shared<Node>(this));
}

void Node::removeChild(std::shared_ptr<Node> child) {
    auto idx = this->_getIndexOfChild(child);
    if (idx > 0) m_children.erase(m_children.begin() + idx);
    child->m_parent = nullptr;
}

void Node::removeChildByTag(int64_t tag) {
    auto child = this->getChildByTag(tag);
    if (child) this->removeChild(child);
}

void Node::removeAllChildren() {
    std::ranges::for_each(
        m_children,
        [this](std::shared_ptr<Node> child) { this->removeChild(child); }
    );
}

void Node::reorderChild(std::shared_ptr<Node> child, int64_t zOrder) {
    auto idx = this->_getIndexOfChild(child);
    if (idx > 0) m_children.at(idx)->setZOrder(zOrder);
    this->sortAllChildren();
}

void Node::sortAllChildren() {
    std::ranges::sort(
        m_children,
        [](std::shared_ptr<Node> a, std::shared_ptr<Node> b) { return a->getZOrder() < b->getZOrder(); }
    );
}

void Node::setTag(int64_t tag) {
    m_tag = tag;
}

void Node::setUserData(std::shared_ptr<void> userData) {
    m_userData = userData;
}

void Node::cleanup() {
    Scheduler::sharedScheduler()->unscheduleUpdate(this);
    this->removeAllChildren();
}

void Node::draw(const long double dt) {
    std::ranges::for_each(
        m_children,
        [dt](std::shared_ptr<Node> child) { child->draw(dt); }
    );
}

size_t Node::_getIndexOfChild(std::shared_ptr<Node> child) {
    auto find = std::ranges::find(
        m_children,
        child
    );

    if (find == m_children.end()) return -1;
    else return find - m_children.begin();
}