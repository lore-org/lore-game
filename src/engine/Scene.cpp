#include <engine/Scene.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>


#include <engine/Engine.h>

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