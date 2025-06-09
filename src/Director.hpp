#pragma once

#include "Geometry.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "raylib.h"

class Director;

inline Director* g_director;

class Director : public Object {
public:
    Director() : m_transitionStart(GetTime()), m_transitionDuration(0), m_clearColor(WHITE) {}
    virtual ~Director() {
        this->release();
        delete this;
    };

    inline virtual bool init() {
        if (!m_transitionFader) m_transitionFader = RectangleNode::create();
        m_transitionFader->setColor(BLACK);
        m_transitionFader->setAnchorPoint({ 0 });
        return true;
    }

    static Director* create() {
        Director* ret = new Director();
        if (!ret->init()) {
            ret->release();
            return nullptr;
        }
        
        return ret;
    }

    inline static Director* sharedDirector() {
        if (!g_director) g_director = Director::create();
        return g_director;
    }

    inline void pushScene(Scene* scene) {
        m_nextScene = scene;
        this->_replaceSceneWithNext();
    }
    // duration is in seconds
    inline void pushSceneWithTransition(Scene* scene, float duration = 5) {
        m_nextScene = scene;
        this->_replaceSceneWithNext();
    }

    inline void popScene(unsigned int depth = 1) {
        m_sceneStack.pop_back();
    }

    inline void replaceTopScene(Scene* scene) {
        m_displayedScene = scene;
        m_sceneStack.back() = m_displayedScene;
    }
    // duration is in seconds
    inline void replaceTopSceneWithTransition(Scene* scene, float duration = 5) {
        m_displayedScene = scene;
        m_sceneStack.back() = m_displayedScene;
    }

    inline Scene* getTopScene() {
        return m_sceneStack.back();
    }
    inline size_t getNumberOfScenes() {
        return m_sceneStack.size();
    }
    inline std::vector<Scene*> getSceneStack() {
        return m_sceneStack;
    }
    
    inline virtual void draw(float dt) {
        ClearBackground(m_clearColor);
        
        if (m_displayedScene) m_displayedScene->draw(dt);
        
        m_transitionFader->setContentSize(Size(GetScreenWidth(), GetScreenHeight()) / 2);
        m_transitionFader->setOpacity(this->_lerpTime(m_transitionStart, m_transitionDuration, GetTime()) * 255);
        m_transitionFader->draw(dt);
    };
protected:
    std::vector<Scene*> m_sceneStack;

    Scene* m_displayedScene;
    Scene* m_nextScene;

    RectangleNode* m_transitionFader;
    double m_transitionStart;
    double m_transitionDuration;

    Color m_clearColor;

private:
    inline void _replaceSceneWithNext() {
        if (m_nextScene) {
            m_sceneStack.push_back(m_nextScene);
            m_displayedScene = m_nextScene;
        }
        m_nextScene = nullptr;
    }

    template<typename T>
    inline T _lerpTime(T start, T duration, T current) {
        auto elapsed = current - start;
        return std::clamp(elapsed / duration, (T)0, (T)1);
    }
};