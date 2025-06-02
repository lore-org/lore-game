#pragma once

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
    

    virtual void addChild(Node* child);
    virtual void addChild(Node* child, int zOrder);
    virtual void addChild(Node* child, int zOrder, int tag);
    virtual Node* getChildByTag(int tag);
    virtual std::vector<Node*> getChildren();
    
    virtual unsigned int getChildrenCount() const;
    
    virtual void setParent(Node* parent);
    virtual Node* getParent();
    
    virtual void removeFromParent();
    virtual void removeChild(Node* child);
    virtual void removeChildByTag(int tag);
    virtual void removeAllChildren();
    
    virtual void reorderChild(Node* child, int zOrder);
    
    virtual void sortAllChildren();
    
    
    virtual void setTag(int tag) {
        m_tag = tag;
    };
    
    virtual std::any* getUserData() {
        return m_userData;
    };
    virtual void setUserData(std::any* userData) {
        m_userData = userData;
    };


    virtual void cleanup();
    virtual void draw();
    virtual void visit();

    
    Rect getRect() {
        return Rect(m_position, m_contentSize);
    };
    
    virtual void update(float dt) {};

private:    
    void insertChild(Node* child, int z);
    void detachChild(Node* child, bool doCleanup);

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