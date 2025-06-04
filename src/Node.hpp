#pragma once

#include <algorithm>
#include <vector>
#include <any>

#include "Object.hpp"
#include "Data.hpp"
#include "Scheduler.hpp"

class Node : public Object
{
public:
    Node() : m_rotation(.0f), m_scale(1.f), m_position(0, 0),
    m_anchorPoint(.5f, .5f), m_contentSize(0, 0), m_zOrder(0),
    m_parent(nullptr), m_tag(0), m_userData(nullptr), m_visible(true) {};
    virtual ~Node() {
        delete m_userData;
        this->removeAllChildren();
    };
    
    virtual bool init() {
        return true;
    };
    static Node* create() {
        Node* ret = new Node();
        if (!ret->init()) return ret;
        else {
            return nullptr;
            ret->release();
        }
    };
    

    inline virtual void setZOrder(int zOrder) {
        m_zOrder = zOrder;
        if (m_parent) m_parent->reorderChild(this, zOrder);
    };
    inline virtual int getZOrder() {
        return m_zOrder;
    };

    inline virtual void setScale(float scale) {
        m_scale = scale;
    };
    inline virtual float getScale() {
        return m_scale;
    };

    inline virtual void setPosition(const Point& position) {
        m_position = position;
    };
    inline virtual const Point& getPosition() const {
        return m_position;
    };
    inline virtual void setPosition(float x, float y) {
        m_position.x = x;
        m_position.y = y;
    };
    inline virtual void setPositionX(float x) {
        m_position.x = x;
    };
    inline virtual float getPositionX() const {
        return m_position.x;
    };
    inline virtual void setPositionY(float y) {
        m_position.y = y;
    };
    inline virtual float getPositionY() const {
        return m_position.y;
    };

    inline virtual void setAnchorPoint(const Point& anchorPoint) {
        m_anchorPoint = anchorPoint;
    };
    inline virtual const Point& getAnchorPoint() const {
        return m_anchorPoint;
    };
    
    inline virtual void setContentSize(const Size& contentSize) {
        m_contentSize = contentSize;
    };
    inline virtual const Size& getContentSize() const {
        return m_contentSize;
    };

    inline virtual void setVisible(bool visible) {
        m_visible = visible;
    };
    inline virtual bool isVisible() const {
        return m_visible;
    };

    inline virtual void setRotation(float rotation) {
        m_rotation = rotation;
    };
    inline virtual float getRotation() const {
        return m_rotation;
    };
    

    inline virtual void addChild(Node* child) {
        this->addChild(
            child,
            m_children.at(m_children.size() - 1)->getZOrder()
        );
    };
    inline virtual void addChild(Node* child, int zOrder) {
        child->setZOrder(zOrder);
        m_children.push_back(child);
        this->sortAllChildren();
    };
    inline virtual void addChild(Node* child, int zOrder, int tag) {
        this->setTag(tag);
        this->addChild(child, zOrder);
    };
    // returns nullptr if child does not exist
    inline virtual Node* getChildByTag(int tag) {
        auto find = std::find_if(
            m_children.begin(),
            m_children.end(),
            [tag](Node& node) { return node.m_tag == tag; }
        );

        if (find != m_children.end()) return *find;
        return nullptr;
    };
    inline virtual std::vector<Node*> getChildren() {
        return m_children;
    };
    
    inline virtual unsigned int getChildrenCount() const {
        return m_children.size();
    };
    
    inline virtual void setParent(Node* parent) {
        m_parent = parent;
    };
    inline virtual Node* getParent() {
        return m_parent;
    };
    
    virtual void removeFromParent() {
        m_parent->removeChild(this);
        this->cleanup();
    };
    virtual void removeChild(Node* child) {
        child->cleanup();
        child->removeFromParent();
        auto idx = this->_getIndexOfChild(child);
        if (idx > 0) m_children.erase(idx + m_children.begin());
    };
    virtual void removeChildByTag(int tag) {
        auto child = this->getChildByTag(tag);
        if (child) this->removeChild(child);
    };
    inline virtual void removeAllChildren() {
        for (auto& child : m_children) this->removeChild(child);
    };
    
    virtual void reorderChild(Node* child, int zOrder) {
        auto idx = this->_getIndexOfChild(child);
        if (idx > 0) m_children.at(idx)->setZOrder(zOrder);
        this->sortAllChildren();
    };
    
    inline virtual void sortAllChildren() {
        std::sort(
            m_children.begin(),
            m_children.end(),
            [](Node& a, Node& b) { return a.getZOrder() < b.getZOrder(); }
        );
    };
    
    
    inline virtual void setTag(int tag) {
        m_tag = tag;
    };
    
    inline virtual std::any* getUserData() {
        return m_userData;
    };
    inline virtual void setUserData(std::any* userData) {
        m_userData = userData;
    };


    inline virtual void cleanup() {
        Scheduler::sharedScheduler()->unscheduleUpdate(this);
        this->removeAllChildren();
    };
    inline virtual void draw() {};

    
    inline Rect getRect() {
        return Rect(m_position, m_contentSize);
    };
    
    inline virtual void update(float dt) {};

private:    
    // returns -1 if child does not exist
    inline size_t _getIndexOfChild(Node* child) {
        auto find = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&child](Node& node) { return &node == child; }
        );

        if (find == m_children.end()) return -1;
        else return find - m_children.begin();
    }

protected:
    float m_rotation;
    float m_scale;
    Point m_position;
    Point m_anchorPoint;
    Size m_contentSize;
    
    int m_zOrder;
    
    std::vector<Node*> m_children;
    Node* m_parent;
    
    int m_tag;
    std::any* m_userData;

    bool m_visible;
};