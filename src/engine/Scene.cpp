#include <furredengine/Scene.h>

#include <discord-rpc.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <furredengine/config.hpp>
#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>

using namespace FurredEngine;

bool Scene::init() {
    if (!Node::init()) return false;

    this->setContentSize(Engine::sharedInstance()->getWindowSize());
    return true;
}

std::shared_ptr<Scene> Scene::create() {
    auto ret = utils::protected_make_shared<Scene>();

    if (!ret->init()) return nullptr;
    return ret;
}