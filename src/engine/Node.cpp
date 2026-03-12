#include <furredengine/Node.h>

#include <algorithm>
#include <memory>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>

#include <furredengine/Geometry.h>
#include <furredengine/utils.h>
#include <furredengine/Scheduler.h>

using namespace FurredEngine;

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

void Node::setPosition(long double x, long double y) {
    m_position.x = x;
    m_position.y = y;
}

void Node::setAnchorPoint(long double x, long double y) {
    m_anchorPoint.x = x;
    m_anchorPoint.y = y;
}

void Node::setContentSize(long double width, long double height) {
    m_contentSize.width = width;
    m_contentSize.height = height;
}

void Node::setVisible(bool visible) {
    m_visible = visible;
}

void Node::setRotation(long double rotation) {
    m_rotation = rotation;
}

bool Node::containsPoint(FurredEngine::Point point) {
    auto rect = this->getRect();

    auto screenOrigin = MakePoint(
        this->getPositionX() + (this->getContentWidth() * this->getAnchorX()),
        this->getPositionY() + (this->getContentHeight() * this->getAnchorY())
    );

    FurredEngine::Point rotatedRect[4] = {
        FurredEngine::Point(rect.getMinX(), rect.getMaxY()).rotateAroundCenter(screenOrigin, this->getRotation()),
        FurredEngine::Point(rect.getMaxX(), rect.getMaxY()).rotateAroundCenter(screenOrigin, this->getRotation()),
        FurredEngine::Point(rect.getMaxX(), rect.getMinY()).rotateAroundCenter(screenOrigin, this->getRotation()),
        FurredEngine::Point(rect.getMinX(), rect.getMinY()).rotateAroundCenter(screenOrigin, this->getRotation())
    };

    auto x1 = rotatedRect[0].x;
    auto x2 = rotatedRect[1].x;
    auto x3 = rotatedRect[2].x;
    auto x4 = rotatedRect[3].x;

    auto y1 = rotatedRect[0].y;
    auto y2 = rotatedRect[1].y;
    auto y3 = rotatedRect[2].y;
    auto y4 = rotatedRect[3].y;

    auto a1 = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    auto a2 = std::sqrt((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3));
    auto a3 = std::sqrt((x3 - x4) * (x3 - x4) + (y3 - y4) * (y3 - y4));
    auto a4 = std::sqrt((x4 - x1) * (x4 - x1) + (y4 - y1) * (y4 - y1));

    auto b1 = std::sqrt((x1 - point.x) * (x1 - point.x) + (y1 - point.y) * (y1 - point.y));
    auto b2 = std::sqrt((x2 - point.x) * (x2 - point.x) + (y2 - point.y) * (y2 - point.y));
    auto b3 = std::sqrt((x3 - point.x) * (x3 - point.x) + (y3 - point.y) * (y3 - point.y));
    auto b4 = std::sqrt((x4 - point.x) * (x4 - point.x) + (y4 - point.y) * (y4 - point.y));

    auto u1 = (a1 + b1 + b2) / 2.L;
    auto u2 = (a2 + b2 + b3) / 2.L;
    auto u3 = (a3 + b3 + b4) / 2.L;
    auto u4 = (a4 + b4 + b1) / 2.L;

    auto A1 = std::sqrt(u1 * (u1 - a1) * (u1 - b1) * (u1 - b2));
    auto A2 = std::sqrt(u2 * (u2 - a2) * (u2 - b2) * (u2 - b3));
    auto A3 = std::sqrt(u3 * (u3 - a3) * (u3 - b3) * (u3 - b4));
    auto A4 = std::sqrt(u4 * (u4 - a4) * (u4 - b4) * (u4 - b1));

    auto diff = A1 + A2 + A3 + A4 - a1 * a2;

    return diff < 1.L;
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
        [tag](std::shared_ptr<Node>& node) { return node->m_tag == tag; }
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
        [this](std::shared_ptr<Node>& child) { this->removeChild(child); }
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
        [](std::shared_ptr<Node>& a, std::shared_ptr<Node>& b) { return a->getZOrder() < b->getZOrder(); }
    );
}

void Node::setTag(int64_t tag) {
    m_tag = tag;
}

void Node::setUserData(void* userData) {
    m_userData = userData;
}

void Node::cleanup() {
    Scheduler::sharedScheduler()->unscheduleUpdate(this);
    this->removeAllChildren();
}

void Node::draw(const long double dt) {
    std::ranges::for_each(
        m_children,
        [dt](std::shared_ptr<Node>& child) { child->draw(dt); }
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