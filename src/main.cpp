#include <raylib.h>

#include "engine/Director.hpp"
#include "engine/Engine.hpp"
#include "engine/Scene.hpp"
#include "engine/Sprite.hpp"

int main() {
    SetupEngine();

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);
    kitty->setUpdate([&kitty](auto dt) {
        kitty->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2); // make sure kitty is in the center of the screen
    });
    kitty->scheduleSelf();

    scene->addChild(kitty);

    Director::sharedDirector()->pushScene(scene);

    RunEngine();
}