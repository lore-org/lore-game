#include "engine/Default.hpp"

#include "engine/Director.hpp"
#include "engine/Engine.hpp"
#include "engine/Scene.hpp"
#include "engine/Sprite.hpp"

int main() {
    SetupEngine();

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setUpdate([&kitty](auto dt) {
        kitty->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2); // make sure kitty is in the center of the screen
        kitty->setRotation(kitty->getRotation() + (45 * dt)); // spin kitty 45deg / second
    });
    kitty->scheduleSelf();

    scene->addChild(kitty);

    Director::sharedDirector()->pushScene(scene);

    RunEngine();
}