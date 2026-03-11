#include <furredengine/Director.h>

#include <thread>

#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>
#include <furredengine/Object.h>
#include <furredengine/Scene.h>
#include <furredengine/Engine.h>
#include <furredengine/RectangleNode.h>

using namespace FurredEngine;

std::shared_ptr<Director> Director::m_instance;

Director::Director() :
    m_transitionStart(Engine::getTimeNS()), m_transitionDuration(0),
    m_entering(false),
    m_clearColor({ 0, 0, 0, 255 }) {}

bool Director::init() {
    if (!Object::init()) return false;

    m_transitionFader = RectangleNode::create();
    m_transitionFader->setColorA(0, 0, 0, 0);
    m_transitionFader->setAnchorPoint(0);
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

void Director::pushSceneWithTransition(std::shared_ptr<Scene> scene, long double duration) {
    m_nextScene = scene;
    this->_transitionBetweenScenes(duration);
}

void Director::popScene(uint64_t depth) {
    if (depth > m_sceneStack.size()) depth = m_sceneStack.size();
    m_nextScene = *(m_sceneStack.end() - depth);

    while (depth > 0 && m_sceneStack.size() > 0) {
        --depth;
        if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    }

    this->_replaceSceneWithNext();
}

void Director::popSceneWithTransition(uint64_t depth, long double duration) {
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

void Director::replaceTopSceneWithTransition(std::shared_ptr<Scene> scene, long double duration) {
    m_nextScene = scene;
    if (m_sceneStack.size() > 0) m_sceneStack.pop_back();
    this->_transitionBetweenScenes(duration);
}

std::shared_ptr<Scene> Director::getTopScene() {
    if (m_sceneStack.size() > 0) return m_sceneStack.back();
    return nullptr;
}

void Director::setClearColor(ColorNode::Color3 clearColor) {
    m_clearColor.r = clearColor.r;
    m_clearColor.g = clearColor.g;
    m_clearColor.b = clearColor.b;
}

void Director::setClearColorA(ColorNode::Color4 clearColor) {
    m_clearColor = clearColor;
}

void Director::setClearOpacity(uint8_t clearOpacity) {
    m_clearColor.a = clearOpacity;
}

void Director::draw(const long double dt) {
    auto engine = Engine::sharedInstance();
    

    glClearColor(
        m_clearColor.r / 255.,
        m_clearColor.g / 255.,
        m_clearColor.b / 255.,
        m_clearColor.a / 255.
    );
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (m_displayedScene) m_displayedScene->draw(dt);
    
    m_transitionFader->setContentSize(engine->getWindowSize());

    auto normalisedOpacity = m_entering ?
        this->_lerpTime(m_transitionStart, m_transitionDuration, static_cast<long double>(Engine::getTimeNS())) :
        std::abs(1 - this->_lerpTime(m_transitionStart, m_transitionDuration, static_cast<long double>(Engine::getTimeNS())));
    m_transitionFader->setOpacity(static_cast<uint8_t>(normalisedOpacity * 255));

    m_transitionFader->draw(dt);
}

void Director::_transitionBetweenScenes(long double duration) {
    std::thread _transitionThread([this, duration]() {
        m_transitionDuration = duration * NanosecondsPerSecond;

        m_entering = true;
        m_transitionStart = Engine::getTimeNS();

        Engine::preciseNanosecondDelay(m_transitionDuration);
        this->_replaceSceneWithNext();

        m_entering = false;
        m_transitionStart = Engine::getTimeNS();
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