#pragma once

#include <vector>

#include "Object.h"
#include "Scene.h"
#include "RectangleNode.h"

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
    std::shared_ptr<Scene> getDisplayedScene();
    size_t getNumberOfScenes();
    std::vector<std::shared_ptr<Scene>> getSceneStack();
    
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