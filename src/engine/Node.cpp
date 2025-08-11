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

void Node::setZOrder(long long zOrder) {
    m_zOrder = zOrder;
    if (m_parent) m_parent->sortAllChildren();
}

long long Node::getZOrder() {
    return m_zOrder;
}

void Node::setScale(double scale) {
    m_scale = scale;
}

double Node::getScale() {
    return m_scale;
}

void Node::setPosition(Point position) {
    m_position = position;
}

Point Node::getPosition() const {
    return m_position;
}

void Node::setPosition(double x, double y) {
    m_position.x = x;
    m_position.y = y;
}

void Node::setPositionX(double x) {
    m_position.x = x;
}

double Node::getPositionX() const {
    return m_position.x;
}

void Node::setPositionY(double y) {
    m_position.y = y;
}

double Node::getPositionY() const {
    return m_position.y;
}

void Node::setAnchorPoint(Point anchorPoint) {
    m_anchorPoint = anchorPoint;
}

Point Node::getAnchorPoint() const {
    return m_anchorPoint;
}

void Node::setContentSize(Size contentSize) {
    m_contentSize = contentSize;  // TODO - implement NULL for filling render target
}

Size Node::getContentSize() const {
    return m_contentSize;
}

void Node::setVisible(bool visible) {
    m_visible = visible;
}

bool Node::isVisible() const {
    return m_visible;
}

void Node::setRotation(double rotation) {
    m_rotation = rotation;
}

double Node::getRotation() const {
    return m_rotation;
}

void Node::addChild(std::shared_ptr<Node> child) {
    child->m_parent = utils::cast_shared<Node>(this);
    m_children.push_back(child);
    this->sortAllChildren();
}

void Node::addChild(std::shared_ptr<Node> child, long long zOrder) {
    child->setZOrder(zOrder);
    this->addChild(child);
}

void Node::addChild(std::shared_ptr<Node> child, long long zOrder, long long tag) {
    this->setTag(tag);
    this->addChild(child, zOrder);
}

std::shared_ptr<Node> Node::getChildByTag(long long tag) {
    auto find = std::ranges::find_if(
        m_children,
        [tag](std::shared_ptr<Node> node) { return node->m_tag == tag; }
    );

    if (find != m_children.end()) return *find;
    return nullptr;
}

std::vector<std::shared_ptr<Node>> Node::getChildren() {
    return m_children;
}

unsigned long long Node::getChildrenCount() const {
    return m_children.size();
}

void Node::setParent(std::shared_ptr<Node> parent) {
    if (m_parent) m_parent->removeChild(utils::cast_shared<Node>(this));
    parent->addChild(utils::cast_shared<Node>(this));
    m_parent = parent;
}

std::shared_ptr<Node> Node::getParent() {
    return m_parent;
}

void Node::removeFromParent() {
    m_parent->removeChild(utils::cast_shared<Node>(this));
}

void Node::removeChild(std::shared_ptr<Node> child) {
    auto idx = this->_getIndexOfChild(child);
    if (idx > 0) m_children.erase(m_children.begin() + idx);
    child->m_parent = nullptr;
}

void Node::removeChildByTag(long long tag) {
    auto child = this->getChildByTag(tag);
    if (child) this->removeChild(child);
}

void Node::removeAllChildren() {
    std::ranges::for_each(
        m_children,
        [this](std::shared_ptr<Node> child) { this->removeChild(child); }
    );
}

void Node::reorderChild(std::shared_ptr<Node> child, long long zOrder) {
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

void Node::setTag(long long tag) {
    m_tag = tag;
}

std::shared_ptr<void> Node::getUserData() {
    return m_userData;
}

void Node::setUserData(std::shared_ptr<void> userData) {
    m_userData = userData;
}

void Node::cleanup() {
    Scheduler::sharedScheduler()->unscheduleUpdate(this);
    this->removeAllChildren();
}

void Node::draw(const double dt) {
    std::ranges::for_each(
        m_children,
        [dt](std::shared_ptr<Node> child) { child->draw(dt); }
    );
}

Rect Node::getRect() {
    return Rect(m_position, m_contentSize);
}

size_t Node::_getIndexOfChild(std::shared_ptr<Node> child) {
    auto find = std::ranges::find(
        m_children,
        child
    );

    if (find == m_children.end()) return -1;
    else return find - m_children.begin();
}