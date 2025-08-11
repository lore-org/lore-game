#pragma once

#include <vector>

#include "Object.h"
#include <engine/Geometry.h>

class Node : public Object {
public:
    virtual ~Node();

    static std::shared_ptr<Node> create();


    virtual void setZOrder(long long zOrder);
    virtual long long getZOrder();

    virtual void setScale(double scale);
    virtual double getScale();

    virtual void setPosition(Point position);
    virtual Point getPosition() const;
    virtual void setPosition(double x, double y);
    virtual void setPositionX(double x);
    virtual double getPositionX() const;
    virtual void setPositionY(double y);
    virtual double getPositionY() const;

    virtual void setAnchorPoint(Point anchorPoint);
    virtual Point getAnchorPoint() const;
    
    virtual void setContentSize(Size contentSize);
    virtual Size getContentSize() const;

    virtual void setVisible(bool visible);
    virtual bool isVisible() const;

    virtual void setRotation(double rotation);
    virtual double getRotation() const;
    

    virtual void addChild(std::shared_ptr<Node> child);
    virtual void addChild(std::shared_ptr<Node> child, long long zOrder);
    virtual void addChild(std::shared_ptr<Node> child, long long zOrder, long long tag);
    // Returns nullptr if child does not exist
    virtual std::shared_ptr<Node> getChildByTag(long long tag);
    virtual std::vector<std::shared_ptr<Node>> getChildren();
    
    virtual unsigned long long getChildrenCount() const;
    
    virtual void setParent(std::shared_ptr<Node> parent);
    virtual std::shared_ptr<Node> getParent();
    
    virtual void removeFromParent();
    virtual void removeChild(std::shared_ptr<Node> child);
    virtual void removeChildByTag(long long tag);
    virtual void removeAllChildren();
    
    virtual void reorderChild(std::shared_ptr<Node> child, long long zOrder);
    
    virtual void sortAllChildren();
    
    
    virtual void setTag(long long tag);
    
    virtual std::shared_ptr<void> getUserData();
    virtual void setUserData(std::shared_ptr<void> userData);


    virtual void cleanup();
    virtual void draw(const double dt);

    
    Rect getRect();

protected:
    Node();

    double m_rotation;
    double m_scale;
    Point m_position;
    Point m_anchorPoint;
    Size m_contentSize;
    
    long long m_zOrder;
    
    std::vector<std::shared_ptr<Node>> m_children;
    std::shared_ptr<Node> m_parent;
    
    long long m_tag;
    std::shared_ptr<void> m_userData;

    bool m_visible;

private:
    // Returns -1 if child does not exist
    virtual size_t _getIndexOfChild(std::shared_ptr<Node> child);
};