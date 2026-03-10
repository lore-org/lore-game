#pragma once

#include <vector>

#include <furredengine/Object.h>
#include <furredengine/Scene.h>
#include <furredengine/RectangleNode.h>

namespace FurredEngine {

class Director : public Object {
public:
    virtual bool init() override;

    static std::shared_ptr<Director> create();

    static std::shared_ptr<Director> sharedDirector();

    void pushScene(std::shared_ptr<Scene> scene);
    // Duration is in seconds
    void pushSceneWithTransition(std::shared_ptr<Scene> scene, long double duration = 5);

    void popScene(uint64_t depth = 1);
    // Duration is in seconds
    void popSceneWithTransition(uint64_t depth = 1, long double duration = 5);

    void replaceTopScene(std::shared_ptr<Scene> scene);
    // Duration is in seconds
    void replaceTopSceneWithTransition(std::shared_ptr<Scene> scene, long double duration = 5);

    std::shared_ptr<Scene> getTopScene();
    inline std::shared_ptr<Scene> getDisplayedScene() { return m_displayedScene; }
    inline size_t getNumberOfScenes() { return m_sceneStack.size(); }
    inline std::vector<std::shared_ptr<Scene>> getSceneStack() { return m_sceneStack; }

    void setClearColor(ColorNode::Color3 clearColor);
    inline ColorNode::Color3 getClearColor() { return { m_clearColor.r, m_clearColor.g, m_clearColor.b }; }

    void setClearColorA(ColorNode::Color4 clearColor);
    inline ColorNode::Color4 getClearColorA() { return m_clearColor; }

    void setClearOpacity(uint8_t clearOpacity);
    inline uint8_t getClearOpacity() { return m_clearColor.a; }
    
    virtual void draw(const long double dt);

protected:
    Director();

    std::vector<std::shared_ptr<Scene>> m_sceneStack;

    std::shared_ptr<Scene> m_displayedScene;
    std::shared_ptr<Scene> m_nextScene;

    std::shared_ptr<RectangleNode> m_transitionFader;
    long double m_transitionStart;
    long double m_transitionDuration;
    bool m_entering;

    ColorNode::Color4 m_clearColor;

private:
    static std::shared_ptr<Director> m_instance;

    void _transitionBetweenScenes(long double duration = 5);

    void _replaceSceneWithNext();

    template<typename T>
    T _lerpTime(T start, T duration, T current);
};

}