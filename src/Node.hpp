#pragma once

#include <vector>
#include <functional>

#include "Object.hpp"
#include "Data.hpp"

/** @brief Node is the main element. Anything that gets drawn or contains things that get drawn is a Node.
 The most popular Nodes are: CCScene, CCLayer, CCSprite, CCMenu.

 The main features of a Node are:
 - They can contain other Node nodes (addChild, getChildByTag, removeChild, etc)
 - They can schedule periodic callback (schedule, unschedule, etc)
 - They can execute actions (runAction, stopAction, etc)

 Some Node nodes provide extra functionality for them or their children.

 Subclassing a Node usually means (one/all) of:
 - overriding init to initialize resources and schedule callbacks
 - create callbacks to handle the advancement of time
 - overriding draw to render the node

 Features of Node:
 - position
 - scale (x, y)
 - rotation (in degrees, clockwise)
 - CCCamera (an interface to gluLookAt )
 - CCGridBase (to do mesh transformations)
 - anchor point
 - size
 - visible
 - z-order
 - openGL z position

 Default values:
 - rotation: 0
 - position: (x=0,y=0)
 - scale: (x=1,y=1)
 - contentSize: (x=0,y=0)
 - anchorPoint: (x=0,y=0)

 Limitations:
 - A Node is a "void" object. It doesn't have a texture

 Order in transformations with grid disabled
 -# The node will be translated (position)
 -# The node will be rotated (rotation)
 -# The node will be scaled (scale)
 -# The node will be moved according to the camera values (camera)

 Order in transformations with grid enabled
 -# The node will be translated (position)
 -# The node will be rotated (rotation)
 -# The node will be scaled (scale)
 -# The grid will capture the screen
 -# The node will be moved according to the camera values (camera)
 -# The grid will render the captured screen

 Camera:
 - Each node has a camera. By default it points to the center of the Node.
 */

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
    
    
    #define SELECTOR std::function<void(float)>
    
    bool isScheduled(SELECTOR selector);

    void scheduleUpdate();

    void scheduleUpdateWithPriority(int priority);

    void unscheduleUpdate();

    #define REPEAT_FOREVER (UINT_MAX -1)
    // use REPEAT_FOREVER macro to repeat
    void schedule(SELECTOR selector, float interval, unsigned int repeat, float delay);
    
    void schedule(SELECTOR selector, float interval);
    
    void scheduleOnce(SELECTOR selector, float delay);
    
    void schedule(SELECTOR selector);
    
    void unschedule(SELECTOR selector);

    void unscheduleAllSelectors();

    void resumeSchedulerAndActions();
    void pauseSchedulerAndActions();
    
    virtual void update(float delta);

private:
    /// lazy allocs
    void childrenAlloc();
    
    /// helper that reorder a child
    void insertChild(Node* child, int z);
    
    /// Removes a child, call child->onExit(), do cleanup, remove it from children array.
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
    
    // TODO - implement m_scheduler
    // CCScheduler *m_scheduler;

    bool m_bVisible;
};