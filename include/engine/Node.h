#pragma once

#include <cstdint>
#include <vector>

#include "Object.h"
#include <engine/Geometry.h>

class Node : public Object {
public:
    virtual ~Node();

    static std::shared_ptr<Node> create();


    virtual void setZOrder(int64_t zOrder);
    virtual int64_t getZOrder();

    virtual void setScale(long double scale);
    virtual long double getScale();

    virtual void setPosition(Point position);
    virtual Point getPosition() const;
    virtual void setPosition(long double x, long double y);
    virtual void setPositionX(long double x);
    virtual long double getPositionX() const;
    virtual void setPositionY(long double y);
    virtual long double getPositionY() const;

    virtual void setAnchorPoint(Point anchorPoint);
    virtual Point getAnchorPoint() const;
    
    virtual void setContentSize(Size contentSize);
    virtual Size getContentSize() const;

    virtual void setVisible(bool visible);
    virtual bool isVisible() const;

    virtual void setRotation(long double rotation);
    virtual long double getRotation() const;
    

    virtual void addChild(std::shared_ptr<Node> child);
    virtual void addChild(std::shared_ptr<Node> child, int64_t zOrder);
    virtual void addChild(std::shared_ptr<Node> child, int64_t zOrder, int64_t tag);
    // Returns nullptr if child does not exist
    virtual std::shared_ptr<Node> getChildByTag(int64_t tag);
    virtual std::vector<std::shared_ptr<Node>> getChildren();
    
    virtual uint64_t getChildrenCount() const;
    
    virtual void setParent(std::shared_ptr<Node> parent);
    virtual std::shared_ptr<Node> getParent();
    
    virtual void removeFromParent();
    virtual void removeChild(std::shared_ptr<Node> child);
    virtual void removeChildByTag(int64_t tag);
    virtual void removeAllChildren();
    
    virtual void reorderChild(std::shared_ptr<Node> child, int64_t zOrder);
    
    virtual void sortAllChildren();
    
    
    virtual void setTag(int64_t tag);
    
    virtual std::shared_ptr<void> getUserData();
    virtual void setUserData(std::shared_ptr<void> userData);


    virtual void cleanup();
    virtual void draw(const long double dt);

    
    Rect getRect();

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
    virtual size_t _getIndexOfChild(std::shared_ptr<Node> child);
};