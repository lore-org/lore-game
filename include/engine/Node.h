#pragma once

#include <cstdint>
#include <vector>

#include "Object.h"
#include <engine/Geometry.h>

class Node : public Object {
public:
    virtual ~Node();

    static std::shared_ptr<Node> create();


    void setZOrder(int64_t zOrder);
    inline int64_t getZOrder() { return m_zOrder; }

    void setScale(long double scale);
    inline long double getScale() { return m_scale; }

    void setPosition(Point position);
    void setPosition(long double x, long double y);
    inline Point getPosition() const { return m_position; }
    void setPositionX(long double x);
    inline long double getPositionX() const { return m_position.x; }
    void setPositionY(long double y);
    inline long double getPositionY() const { return m_position.y; }

    void setAnchorPoint(Point anchorPoint);
    void setAnchorPoint(long double x, long double y);
    inline Point getAnchorPoint() const { return m_anchorPoint; }
    void setAnchorX(long double x);
    inline long double getAnchorX() const { return m_anchorPoint.x; }
    void setAnchorY(long double y);
    inline long double getAnchorY() const { return m_anchorPoint.y; }

    void setContentSize(Size contentSize);
    void setContentSize(long double width, long double height);
    inline Size getContentSize() const { return m_contentSize; }
    void setContentWidth(long double width);
    inline long double getContentWidth() const { return m_contentSize.width; }
    void setContentHeight(long double height);
    inline long double getContentHeight() const { return m_contentSize.height; }

    void setVisible(bool visible);
    inline bool isVisible() const { return m_visible; }

    void setRotation(long double rotation);
    inline long double getRotation() const { return m_rotation; }

    bool containsPoint(Point point);
    

    void addChild(std::shared_ptr<Node> child);
    void addChild(std::shared_ptr<Node> child, int64_t zOrder);
    void addChild(std::shared_ptr<Node> child, int64_t zOrder, int64_t tag);
    // Returns nullptr if child does not exist
    std::shared_ptr<Node> getChildByTag(int64_t tag);
    inline std::vector<std::shared_ptr<Node>> getChildren() { return m_children; }
    
    inline uint64_t getChildrenCount() const { return m_children.size(); }
    
    void setParent(std::shared_ptr<Node> parent);
    inline std::shared_ptr<Node> getParent() { return m_parent; }
    
    void removeFromParent();
    void removeChild(std::shared_ptr<Node> child);
    void removeChildByTag(int64_t tag);
    void removeAllChildren();
    
    void reorderChild(std::shared_ptr<Node> child, int64_t zOrder);
    
    void sortAllChildren();
    
    
    void setTag(int64_t tag);
    
    inline std::shared_ptr<void> getUserData() { return m_userData; }
    void setUserData(std::shared_ptr<void> userData);


    virtual void cleanup();
    virtual void draw(const long double dt);

    
    inline Rect getRect() { return Rect(m_position, m_contentSize); }

protected:
    Node();

    long double m_rotation;
    long double m_scale;
    Point m_position;
    Point m_anchorPoint;
    Size m_contentSize;
    
    int64_t m_zOrder;
    
    std::vector<std::shared_ptr<Node>> m_children;
    std::shared_ptr<Node> m_parent;
    
    int64_t m_tag;
    std::shared_ptr<void> m_userData;

    bool m_visible;

private:
    // Returns -1 if child does not exist
    size_t _getIndexOfChild(std::shared_ptr<Node> child);
};