#include "engine/Director.hpp"
#include "engine/Engine.hpp"
#include "engine/Scene.hpp"
#include "engine/Sprite.hpp"

int main() {
    SetupEngine();

    auto scene = Scene::create();
    auto kitty = Sprite::createFromFile("resources/kitty.png");
    scene->addChild(kitty);
    Director::sharedDirector()->pushScene(scene);

    RunEngine();
}