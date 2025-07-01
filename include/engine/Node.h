#pragma once

#include <vector>

#include "Object.h"
#include "Geometry.h"

class Node : public Object {
public:
    virtual ~Node();

    static std::shared_ptr<Node> create();


    virtual void setZOrder(int zOrder);
    virtual int getZOrder();

    virtual void setScale(float scale);
    virtual float getScale();

    virtual void setPosition(Point position);
    virtual Point getPosition() const;
    virtual void setPosition(float x, float y);
    virtual void setPositionX(float x);
    virtual float getPositionX() const;
    virtual void setPositionY(float y);
    virtual float getPositionY() const;

    virtual void setAnchorPoint(Point anchorPoint);
    virtual Point getAnchorPoint() const;
    
    virtual void setContentSize(Size contentSize);
    virtual Size getContentSize() const;

    virtual void setVisible(bool visible);
    virtual bool isVisible() const;

    virtual void setRotation(float rotation);
    virtual float getRotation() const;
    

    virtual void addChild(std::shared_ptr<Node> child);
    virtual void addChild(std::shared_ptr<Node> child, int zOrder);
    virtual void addChild(std::shared_ptr<Node> child, int zOrder, int tag);
    // Returns nullptr if child does not exist
    virtual std::shared_ptr<Node> getChildByTag(int tag);
    virtual std::vector<std::shared_ptr<Node>> getChildren();
    
    virtual unsigned int getChildrenCount() const;
    
    virtual void setParent(std::shared_ptr<Node> parent);
    virtual std::shared_ptr<Node> getParent();
    
    virtual void removeFromParent();
    virtual void removeChild(std::shared_ptr<Node> child);
    virtual void removeChildByTag(int tag);
    virtual void removeAllChildren();
    
    virtual void reorderChild(std::shared_ptr<Node> child, int zOrder);
    
    virtual void sortAllChildren();
    
    
    virtual void setTag(int tag);
    
    virtual std::shared_ptr<void> getUserData();
    virtual void setUserData(std::shared_ptr<void> userData);


    virtual void cleanup();
    virtual void draw(const double dt);

    
    Rect getRect();

protected:
    Node();

    float m_rotation;
    float m_scale;
    Point m_position;
    Point m_anchorPoint;
    Size m_contentSize;
    
    int m_zOrder;
    
    std::vector<std::shared_ptr<Node>> m_children;
    std::shared_ptr<Node> m_parent;
    
    int m_tag;
    std::shared_ptr<void> m_userData;

    bool m_visible;

private:
    // Returns -1 if child does not exist
    virtual size_t _getIndexOfChild(std::shared_ptr<Node> child);
};