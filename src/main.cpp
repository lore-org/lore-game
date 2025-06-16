#include "raylib.h"

#include "engine/Director.hpp"
#include "engine/Engine.hpp"
#include "engine/Scene.hpp"
#include "engine/Sprite.hpp"

int main() {
    SetupEngine();

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);

    scene->addChild(kitty);

    Director::sharedDirector()->pushScene(scene);

    RunEngine();
}