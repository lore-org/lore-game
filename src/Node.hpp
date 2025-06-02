#pragma once

#include <vector>

#include "Object.hpp"
#include "Data.hpp"
#include "Scheduler.hpp"

class Node : public Object
{
public:
    Node();
    virtual ~Node();
    
    virtual bool init();
    static Node* create();
    
    const char* description();
    

    virtual void setZOrder(int zOrder);
    virtual int getZOrder();

    virtual void setScale(float scale);
    virtual float getScale();

    virtual void setScale(float fScaleX,float fScaleY);

    virtual void setPosition(const Point &position);
    virtual const Point& getPosition();
    virtual void setPosition(float x, float y);
    virtual void getPosition(float* x, float* y);
    virtual void setPositionX(float x);
    virtual float getPositionX();
    virtual void setPositionY(float y);
    virtual float getPositionY();

    virtual void setAnchorPoint(const Point& anchorPoint);
    virtual const Point& getAnchorPoint();
    
    virtual void setContentSize(const Size& contentSize);
    virtual const Size& getContentSize() const;

    virtual void setVisible(bool visible);
    virtual bool isVisible();

    virtual void setRotation(float fRotation);
    virtual float getRotation();
    

    virtual void addChild(Node* child);
    virtual void addChild(Node* child, int zOrder);
    virtual void addChild(Node* child, int zOrder, int tag);
    virtual Node* getChildByTag(int tag);
    virtual std::vector<Node*> getChildren();
    
    virtual unsigned int getChildrenCount() const;
    
    virtual void setParent(Node* parent);
    virtual Node* getParent();
    
    virtual void removeFromParent();
    virtual void removeFromParentAndCleanup(bool cleanup);
    virtual void removeChild(Node* child);
    virtual void removeChild(Node* child, bool cleanup);
    virtual void removeChildByTag(int tag);
    virtual void removeChildByTag(int tag, bool cleanup);
    virtual void removeAllChildren();
    virtual void removeAllChildrenWithCleanup(bool cleanup);
    
    virtual void reorderChild(Node * child, int zOrder);
    
    virtual void sortAllChildren();
    
    
    virtual void setTag(int nTag);
    
    virtual void* getUserData();
    virtual void setUserData(void* userData);


    virtual void onEnter();
    virtual void onEnterTransitionDidFinish();

    virtual void onExit();
    virtual void onExitTransitionDidStart();

    virtual void cleanup();
    virtual void draw();
    virtual void visit();

    
    Rect boundingBox();
    
    virtual void update(float delta);

private:    
    void insertChild(Node* child, int z);
    void detachChild(Node *child, bool doCleanup);

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

    bool m_bVisible;
};