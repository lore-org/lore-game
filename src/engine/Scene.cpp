#include <engine/Default.h>

#include <engine/Scene.h>

#include <engine/Engine.h>
#include <engine/utils.hpp>

bool Scene::init() {
    if (!Node::init()) return false;

    this->setContentSize(Engine::sharedInstance()->getWindowSize());
    return true;
}

std::shared_ptr<Scene> Scene::create() {
    auto ret = utils::protected_make_shared<Scene>();

    if (!ret->init()) return nullptr;
    return ret;
};