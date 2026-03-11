#pragma once

#include <cstdint>
#include <vector>

#include <furredengine/Object.h>
#include <furredengine/utils.h>
#include <furredengine/Geometry.h>

namespace FurredEngine {

class Node : public Object {
public:
    virtual ~Node();

    static std::shared_ptr<Node> create();


    void setZOrder(int64_t zOrder);
    inline int64_t getZOrder() { return m_zOrder; }

    virtual void setScale(long double scale);
    inline long double getScale() { return m_scale; }


    virtual void setPosition(long double x, long double y);

    virtual inline void setPosition(FurredEngine::Point position) { setPosition(position.x, position.y); }
    inline void setPositionX(long double x) { setPosition(x, getPositionY()); }
    inline void setPositionY(long double y) { setPosition(getPositionX(), y); }

    inline FurredEngine::Point getPosition() const { return m_position; }
    inline long double getPositionX() const { return m_position.x; }
    inline long double getPositionY() const { return m_position.y; }


    virtual void setAnchorPoint(long double x, long double y);

    virtual inline void setAnchorPoint(FurredEngine::Point anchorPoint) { setAnchorPoint(anchorPoint.x, anchorPoint.y); }
    inline void setAnchorX(long double x) { setAnchorPoint(x, getAnchorY()); }
    inline void setAnchorY(long double y) { setAnchorPoint(getAnchorX(), y); }

    inline FurredEngine::Point getAnchorPoint() const { return m_anchorPoint; }
    inline long double getAnchorX() const { return m_anchorPoint.x; }
    inline long double getAnchorY() const { return m_anchorPoint.y; }


    virtual void setContentSize(long double width, long double height);

    virtual inline void setContentSize(FurredEngine::Size contentSize) { setContentSize(contentSize.width, contentSize.height); }
    inline void setContentWidth(long double width) { setContentSize(width, getContentHeight()); }
    inline void setContentHeight(long double height) { setContentSize(getContentWidth(), height); }

    inline FurredEngine::Size getContentSize() const { return m_contentSize; }
    inline long double getContentWidth() const { return m_contentSize.width; }
    inline long double getContentHeight() const { return m_contentSize.height; }


    void setVisible(bool visible);
    inline bool isVisible() const { return m_visible; }

    virtual void setRotation(long double rotation);
    inline long double getRotation() const { return m_rotation; }

    bool containsPoint(FurredEngine::Point point);
    

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
    
    inline void* getUserData() { return m_userData; }
    void setUserData(void* userData);


    virtual void cleanup();
    virtual void draw(const long double dt);

    
    inline FurredEngine::Rect getRect() {
        auto scaledWidth = this->getContentWidth() * this->getScale();
        auto scaledHeight = this->getContentHeight() * this->getScale();

        auto xOffset = IsZero(this->getAnchorX()) ? 0 : scaledWidth * this->getAnchorX();
        auto yOffset = IsZero(this->getAnchorY()) ? 0 : scaledHeight * this->getAnchorY();

        return {
            this->getPositionX() - xOffset,
            this->getPositionY() - yOffset,
            scaledWidth,
            scaledHeight
        };
    }

protected:
    Node();

    long double m_rotation;
    long double m_scale;
    FurredEngine::Point m_position;
    FurredEngine::Point m_anchorPoint;
    FurredEngine::Size m_contentSize;
    
    int64_t m_zOrder;
    
    std::vector<std::shared_ptr<Node>> m_children;
    std::shared_ptr<Node> m_parent;
    
    int64_t m_tag;
    void* m_userData;

    bool m_visible;

private:
    // Returns -1 if child does not exist
    size_t _getIndexOfChild(std::shared_ptr<Node> child);
};

}