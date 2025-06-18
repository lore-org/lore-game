#pragma once
#include "Default.hpp"

#include <algorithm>
#include <vector>

#include "Object.hpp"
#include "Geometry.hpp"
#include "Scheduler.hpp"
#include "utils.hpp"

class Node : public Object {
public:
    Node() : m_rotation(.0f), m_scale(1.f), m_position(0),
    m_anchorPoint(.5f), m_contentSize(0), m_zOrder(0),
    m_parent(nullptr), m_tag(0), m_userData(nullptr), m_visible(true) {};
    virtual ~Node() {
        this->removeAllChildren();
        this->release();
        delete this;
    };

    static Node* create() {
        auto ret = new Node();
        if (!ret->init()) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }
    

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
            m_children.empty() ? 0 : m_children.back()->getZOrder()
        );
    };
    inline virtual void addChild(Node* child, int zOrder) {
        child->setZOrder(zOrder);
        child->m_parent = this;
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
            [tag](Node* node) { return node->m_tag == tag; }
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
        if (m_parent) m_parent->removeChild(this);
        parent->addChild(this);
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
        child->removeFromParent();
        auto idx = this->_getIndexOfChild(child);
        if (idx > 0) m_children.erase(m_children.begin() + idx);
    };
    virtual void removeChildByTag(int tag) {
        auto child = this->getChildByTag(tag);
        if (child) this->removeChild(child);
    };
    inline virtual void removeAllChildren() {
        std::for_each(
            m_children.begin(),
            m_children.end(),
            [this](Node* child) { this->removeChild(child); }
        );
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
            [](Node* a, Node* b) { return a->getZOrder() < b->getZOrder(); }
        );
    };
    
    
    inline virtual void setTag(int tag) {
        m_tag = tag;
    };
    
    inline virtual void* getUserData() {
        return m_userData;
    };
    inline virtual void setUserData(void* userData) {
        m_userData = userData;
    };


    inline virtual void cleanup() {
        Scheduler::sharedScheduler()->unscheduleUpdate(this);
        this->removeAllChildren();
        this->release();
    };
    virtual void draw(double dt) {
        std::for_each(
            m_children.begin(),
            m_children.end(),
            [dt](Node* child) { child->draw(dt); }
        );
    };

    
    inline Rect getRect() {
        return Rect(m_position, m_contentSize);
    };

private:    
    // returns -1 if child does not exist
    inline virtual size_t _getIndexOfChild(Node* child) {
        auto find = std::find(
            m_children.begin(),
            m_children.end(),
            child
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
    void* m_userData;

    bool m_visible;
};

class ColorNode : public Node {
public:
    ColorNode() : m_color(WHITE) {};

    static ColorNode* create() {
        auto ret = new ColorNode();
        if (!ret->init()) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    inline void setOpacity(unsigned char opacity) {
        m_color.a = opacity;
    }
    inline int8_t getOpacity() {
        return m_color.a;
    }

    struct Color3 {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    inline void setColor(Color3 color) {
        m_color.r = color.r;
        m_color.g = color.g;
        m_color.b = color.b;
    }
    // use Raylib's 'Color' struct
    inline void setRayColor(Color color) {
        m_color = color;
    }
    inline Color3 getColor() {
        return { m_color.r, m_color.g, m_color.b };
    }
    // use Raylib's 'Color' struct
    inline Color getRayColor() {
        return m_color;
    }

protected:
    Color m_color;
};



class RectangleNode : public ColorNode {
public:
    virtual bool init(Point origin, Size size) {
        if (!ColorNode::init()) return false;

        this->setPosition(origin);
        this->setContentSize(size);
        return true;
    }

    static RectangleNode* create() {
        auto ret = new RectangleNode();
        if (!ret->init(0, 0)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    static RectangleNode* createWithVec(Point origin, Size size) {
        auto ret = new RectangleNode();
        if (!ret->init(origin, size)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    static RectangleNode* createWithRect(Rect rectangle) {
        auto ret = new RectangleNode();
        if (!ret->init(rectangle.origin, rectangle.size)) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    virtual void draw(double dt) override {
        auto xOffset = IsZero(m_anchorPoint.x) ? 0 : m_contentSize.width * m_anchorPoint.x;
        auto yOffset = IsZero(m_anchorPoint.y) ? 0 : m_contentSize.height * m_anchorPoint.y;
        DrawRectangleV(
            m_position - Point(xOffset, yOffset),
            m_contentSize,
            m_color
        );

        ColorNode::draw(dt);
    }
};