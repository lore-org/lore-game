#include <engine/Default.h>

#include <engine/Director.h>

#include <thread>

#include <engine/Object.h>
#include <engine/Scene.h>
#include <engine/Engine.h>
#include <engine/RectangleNode.h>
#include <engine/utils.hpp>

std::shared_ptr<Director> Director::m_instance;

Director::Director() :
    m_transitionStart(Engine::getTime()), m_transitionDuration(0),
    m_clearColor({ 0, 0, 0, 255 }),
    m_entering(false) {};

bool Director::init() {
    if (!Object::init()) return false;

    m_transitionFader = RectangleNode::create();
    m_transitionFader->setColorA({ 0, 0, 0, 0 });
    m_transitionFader->setAnchorPoint({ 0 });
    return true;
}

std::shared_ptr<Director> Director::create() {
    auto ret = utils::protected_make_shared<Director>();

    if (!ret->init()) return nullptr;
    return ret;
}

std::shared_ptr<Director> Director::sharedDirector() {
    if (!Director::m_instance) Director::m_instance = Director::create();
    return Director::m_instance;
}

void Director::pushScene(std::shared_ptr<Scene> scene) {
    m_nextScene = scene;
    this->_replaceSceneWithNext();
}

void Director::pushSceneWithTransition(std::shared_ptr<Scene> scene, float duration) {
    m_nextScene = scene;
    this->_transitionBetweenScenes(duration);
}

void Director::popScene(unsigned int depth) {
    if (depth > m_sceneStack.size()) depth = m_sceneStack.size();
    m_nextScene = *(m_sceneStack.end() - depth);

    while (depth > 0 && m_sceneStack.size() > 0) {
        --depth;
        if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    }

    this->_replaceSceneWithNext();
}

void Director::popSceneWithTransition(unsigned int depth, float duration) {
    if (depth > m_sceneStack.size()) depth = m_sceneStack.size();
    m_nextScene = *(m_sceneStack.end() - depth);

    while (depth > 0 && m_sceneStack.size() > 0) {
        --depth;
        if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    }

    this->_transitionBetweenScenes(duration);
}

void Director::replaceTopScene(std::shared_ptr<Scene> scene) {
    m_nextScene = scene;
    if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    this->_replaceSceneWithNext();
}

void Director::replaceTopSceneWithTransition(std::shared_ptr<Scene> scene, float duration) {
    m_nextScene = scene;
    if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    this->_transitionBetweenScenes(duration);
}

std::shared_ptr<Scene> Director::getTopScene() {
    if (m_sceneStack.size() > 0) return m_sceneStack.back();
    return nullptr;
}

std::shared_ptr<Scene> Director::getDisplayedScene() {
    return m_displayedScene;
}

size_t Director::getNumberOfScenes() {
    return m_sceneStack.size();
}

std::vector<std::shared_ptr<Scene>> Director::getSceneStack() {
    return m_sceneStack;
}

void Director::draw(const double dt) {
    auto renderer = Engine::sharedInstance()->getRenderer();

    if (!SDL_SetRenderDrawColor(
        renderer,
        m_clearColor.r,
        m_clearColor.g,
        m_clearColor.b,
        m_clearColor.a
    )) {
        PrintSDLError();
        return;
    }
    if (!SDL_RenderClear(renderer)) {
        PrintSDLError();
        return;
    };
    
    if (m_displayedScene) m_displayedScene->draw(dt);
    
    m_transitionFader->setContentSize(Engine::sharedInstance()->getWindowSize());

    auto normalisedOpacity = m_entering ?
        this->_lerpTime(m_transitionStart, m_transitionDuration, static_cast<double>(Engine::getTime())) :
        std::abs(1 - this->_lerpTime(m_transitionStart, m_transitionDuration, static_cast<double>(Engine::getTime())));
    m_transitionFader->setOpacity(normalisedOpacity * 255);

    // M_transitionFader->draw(dt);

    if (!SDL_RenderPresent(renderer)) {
        PrintSDLError();
        return;
    };
};

void Director::_transitionBetweenScenes(float duration) {
    std::thread _transitionThread([this, duration]() {
        m_transitionDuration = duration;

        m_entering = true;
        m_transitionStart = Engine::getTime();

        SDL_DelayPrecise(m_transitionDuration);
        this->_replaceSceneWithNext();

        m_entering = false;
        m_transitionStart = Engine::getTime();
    });
}

void Director::_replaceSceneWithNext() {
    if (m_nextScene) {
        m_sceneStack.push_back(m_nextScene);
        m_displayedScene = m_nextScene;
    }
    m_nextScene = nullptr;
}

template<typename T>
T Director::_lerpTime(T start, T duration, T current) {
    auto elapsed = current - start;
    return std::clamp(elapsed / duration, static_cast<T>(0), static_cast<T>(1));
}