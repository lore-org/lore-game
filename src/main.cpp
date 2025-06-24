#include "engine/Default.hpp" // IWYU pragma: keep

#include <curl/curl.h>

#include "engine/Director.hpp"
#include "engine/Engine.hpp"
#include "engine/PresenceManager.hpp"
#include "engine/Scene.hpp"
#include "engine/Sprite.hpp"

int main() {
    auto engine = Engine::sharedInstance();
    auto presence = utils::PresenceManager::sharedManager();

    // ---- Config Setup ----

    engine->showFPS(true);
    engine->showTPS(true);
    engine->setTimeDisplaySampleSize(25);
    engine->setScreenSize({ 480, 320 });

    presence->enableRPC(true);

    // -----------------------------

    /**
     * This function will initialise the Game Window along with the Discord RPC manager
     * if enabled.
     * 
     * It must be called before any graphics are created, otherwise the RAM and VRAM
     * buffers will not be initialised.
     * 
     * Only the PresenceManager and the Screen Size must be set before setup.
     * Running this more than once will print a warning, but will not crash the program.
     */
    Engine::sharedInstance()->setupEngine();

    // ---- User-Defined Code ----

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setUpdate([&kitty](auto dt) {
        kitty->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2); // make sure kitty is in the center of the screen
        kitty->setRotation(kitty->getRotation() + (45 * dt)); // spin kitty 45deg / second
    });
    kitty->scheduleSelf();

    scene->addChild(kitty);

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

    // ---- Cleanup ----

    /**
     * Technically, freeing the parent scene is not required as memory should be freed by
     * the operating system when execution is finished, but it's good practice to remove
     * any heap-allocated variables.
     */
    delete scene;

    // -----------------
}