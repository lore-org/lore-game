#define SDL_MAIN_HANDLED
#define SDL_MAIN_NEEDED
#include <SDL3/SDL_main.h>

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
#include <engine/PresenceManager.h>
#include <engine/Scene.h>
#include <engine/Sprite.h>
#include <engine/Director.h>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    auto engine = Engine::sharedInstance();
    auto presence = utils::PresenceManager::sharedManager();

    // ---- Config Setup ----

    engine->showFPS(true);
    engine->showTPS(true);
    engine->setTimeDisplaySampleSize(25);
    // engine->setFramesPerSecond(30);

    presence->enableRPC(true);

    // -----------------------------

    /**
     * This function will initialise the Game Window along with the Discord RPC manager
     * if enabled.
     * 
     * It must be called before any graphics are created, otherwise the buffers will not be initialised.
     * 
     * Only the PresenceManager and the Screen Size must be set before setup.
     * Running this more than once will print a warning, but will not crash the program.
     */
    Engine::sharedInstance()->setupEngine();

    // ---- User-Defined Code ----

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setUpdate(std::make_shared<Update_Callback>([&kitty](auto dt) {
        kitty->setPosition(Engine::sharedInstance()->getStaticWindowSize() / 2.f); // Make sure kitty is in the center of the screen
        kitty->setRotation(kitty->getRotation() + (45 * dt)); // Spin kitty 45deg / second
    }));
    // kitty->scheduleSelf();

    // scene->addChild(kitty);

    auto furries = Sprite::createFromURL("https://static1.e621.net/data/a8/f2/a8f216299b6b4a83d6d8bf038300a0d0.jpg");
    furries->setAnchorPoint(0);
    furries->setZOrder(-1);
    scene->addChild(furries);

    Director::sharedDirector()->pushScene(scene);

    // ---------------------------

    /**
     * This function creates the thread used by the Scheduler, and runs the main loop
     * each frame.
     * 
     * If this is called before the engine is set up, the program will crash, as the
     * window will not be initialised.
     * 
     * This will pause all execution on the main thread until the window is closed and
     * the background threads are finished. Any code after this should be designated to
     * data saving and cleanup.
     */
    Engine::sharedInstance()->runEngine();

    return 0;
}