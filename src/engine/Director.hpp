#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include <thread>

#include "Geometry.hpp"
#include "Object.hpp"
#include "Scene.hpp"

class Director;

inline Director* g_director;

class Director : public Object {
public:
    Director() : m_transitionStart(GetTime()), m_transitionDuration(0),
        m_clearColor(WHITE), m_entering(false) {}

    virtual bool init() {
        if (!Object::init()) return false;

        m_transitionFader = RectangleNode::create();
        m_transitionFader->setRayColor({ 0, 0, 0, 0 });
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
        this->_transitionBetweenScenes(duration);
    }

    inline void popScene(unsigned int depth = 1) {
        if (depth > m_sceneStack.size()) depth = m_sceneStack.size();
        m_nextScene = *(m_sceneStack.end() - depth);

        while (depth > 0 && m_sceneStack.size() > 0) {
            --depth;
            if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
        }

        this->_replaceSceneWithNext();
    }
    inline void popSceneWithTransition(unsigned int depth = 1, float duration = 5) {
        if (depth > m_sceneStack.size()) depth = m_sceneStack.size();
        m_nextScene = *(m_sceneStack.end() - depth);

        while (depth > 0 && m_sceneStack.size() > 0) {
            --depth;
            if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
        }

        this->_transitionBetweenScenes(duration);
    }

    inline void replaceTopScene(Scene* scene) {
        m_nextScene = scene;
        if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
        this->_replaceSceneWithNext();
    }
    // duration is in seconds
    inline void replaceTopSceneWithTransition(Scene* scene, float duration = 5) {
        m_nextScene = scene;
        if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
        this->_transitionBetweenScenes(duration);
    }

    inline Scene* getTopScene() {
        if (m_sceneStack.size() > 0) return m_sceneStack.back();
        return nullptr;
    }
    inline Scene* getDisplayedScene() {
        return m_displayedScene;
    }
    inline size_t getNumberOfScenes() {
        return m_sceneStack.size();
    }
    inline std::vector<Scene*> getSceneStack() {
        return m_sceneStack;
    }
    
    virtual void draw(const double dt) {
        ClearBackground(m_clearColor);
        
        if (m_displayedScene) m_displayedScene->draw(dt);
        
        m_transitionFader->setContentSize(Size(GetScreenWidth(), GetScreenHeight()));

        auto normalisedOpacity = m_entering ?
            this->_lerpTime(m_transitionStart, m_transitionDuration, GetTime()) :
            std::abs(1 - this->_lerpTime(m_transitionStart, m_transitionDuration, GetTime()));
        m_transitionFader->setOpacity(normalisedOpacity * 255);

        m_transitionFader->draw(dt);
    };
protected:
    std::vector<Scene*> m_sceneStack;

    Scene* m_displayedScene;
    Scene* m_nextScene;

    RectangleNode* m_transitionFader;
    double m_transitionStart;
    double m_transitionDuration;
    bool m_entering;

    Color m_clearColor;

private:
    inline void _transitionBetweenScenes(float duration = 5) {
        new std::thread([this, duration]() {
            m_transitionDuration = duration;

            m_entering = true;
            m_transitionStart = GetTime();

            WaitTime(m_transitionDuration);
            this->_replaceSceneWithNext();

            m_entering = false;
            m_transitionStart = GetTime();
        });
    }

    inline void _replaceSceneWithNext() {
        if (m_nextScene) {
            m_sceneStack.push_back(m_nextScene);
            m_displayedScene = m_nextScene;
        }
        m_nextScene = nullptr;
    }

    template<typename T>
    T _lerpTime(T start, T duration, T current) {
        auto elapsed = current - start;
        return std::clamp(elapsed / duration, (T)0, (T)1);
    }
};