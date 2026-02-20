#include <memory>

#include <discord-rpc.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.h>
#include <engine/PresenceManager.h>
#include <engine/Scene.h>
#include <engine/Sprite.h>
#include <engine/Director.h>
#include <engine/Typeable.h>
#include <engine/RectangleNode.h>
#include <engine/TextNode.h>

int main() {
    auto engine = Engine::sharedInstance();
    auto presence = utils::PresenceManager::sharedManager();

    // ---- Config Setup ----

    engine->setTimeDisplaySampleSize(25);
    // engine->setFramesPerSecond(30);

    presence->enableRPC(true);

    // -----------------------------

    /**
     * This function will initialise the Game Window along with the Discord RPC manager
     * if enabled.
     * 
     * It must be called before any graphics are created, otherwise the buffers will not
     * be initialised.
     * 
     * Only the PresenceManager must be set before setup.
     * Running this more than once will print a warning, but will not crash the program.
     */

    /**
     * If you're familiar with programming concepts, especially with Java, you will
     * recognise this as a singleton!
     * 
     * FurredEngine uses a singleton along with PresenceManager and some other managers.
     * This is to prevent copies of data where there should only be one instance, and
     * provides a centralised area to handle events, rendering, etc.
     * 
     * In addition, all nodes disallow external construction, and instead have their own
     * ::sharedInstance() or ::create(). These use C++'s smart pointers, which assist
     * in destroying memory allocated onto the heap when it is no longer being used.
     */
    Engine::sharedInstance()->setupEngine();

    /**
     * Some config values have to be set AFTER the engine has been set up!
     * This is because they rely on other Nodes that are created during setup, such as
     * FPS and TPS.
     * 
     * If a config value isn't working, try setting it after Engine::setupEngine()!
     */
    engine->showFPS(true);
    engine->showTPS(true);

    // ---- User-Defined Code ----

    auto scene = Scene::create();

    auto kitty = Sprite::createFromFile("resources/kitty.png");
    kitty->setContentSize(100, 100);
    kitty->setUpdate(std::make_shared<Update_Callback>([&kitty](auto dt) {
        kitty->setPosition(Engine::sharedInstance()->getFrameBufferSize() / 2.L); // Make sure kitty is in the center of the screen
        kitty->setRotation(kitty->getRotation() + (45 * dt)); // Spin kitty 45deg / second
    }));
    kitty->scheduleSelf();
    scene->addChild(kitty);

    // auto furries = Sprite::createFromURL("https://static1.e621.net/data/a8/f2/a8f216299b6b4a83d6d8bf038300a0d0.jpg");
    // furries->setAnchorPoint(0);
    // furries->setZOrder(-1);
    // furries->setScale(0.2);
    // furries->setUpdate(std::make_shared<Update_Callback>([&furries](auto dt) {
    //     auto x = cos(Engine::getTimeNS() * SecondsPerNanosecond);
    //     auto y = sin(Engine::getTimeNS() * SecondsPerNanosecond);
    //     furries->setPosition((MakePoint(x, y) * 50) + 100);
    // }));
    // furries->scheduleSelf();
    // scene->addChild(furries);


    auto textInput = Typeable::createWithColors(
        { 30, 25, 40, 255 },
        { 30, 25, 40, 190 },
        { 210, 205, 220, 255 }
    );
    textInput->m_placeholderText->setDisplayedText("Placeholder...");
    textInput->setAnchorPoint(0);
    textInput->setPosition(100, 100);
    textInput->setContentSize(250, 30);
    textInput->setUpdate(std::make_shared<Update_Callback>([&textInput](auto) {
        textInput->m_background->setOpacity(!textInput->isPressed() ? 255 : 200);
    }));
    textInput->scheduleSelf();
    scene->addChild(textInput);

    auto cursor = RectangleNode::create();
    cursor->setContentSize(3);
    cursor->setUpdate(std::make_shared<Update_Callback>([&cursor](auto) {
        auto mouseData = Engine::sharedInstance()->getMouseData();

        cursor->setColor({ static_cast<unsigned char>(mouseData.lmb * 255), 255, static_cast<unsigned char>(mouseData.rmb * 255) });
        cursor->setPosition({ mouseData.x, mouseData.y });

        // LogInfo(fmt::format("   x: {}   y: {}", mouseData.x, mouseData.y));
    }));
    cursor->scheduleSelf();

    scene->addChild(cursor);

    Director::sharedDirector()->pushScene(scene);
    Director::sharedDirector()->setClearColor({ 10, 5, 15 });

    // ---------------------------

    /**
     * This function creates the thread used by the Scheduler, and runs the main loop
     * each frame.
     * 
     * If this is called before the Engine is set up, the program WILL crash, as no
     * contexts, buffers, or instances will have been created.†
     * 
     * This will pause all execution on the main thread until the window is closed and
     * the background threads are finished. Any code after this should be designated to
     * data saving and cleanup.
     * 
     * 
     * 
     * †This is not entirely true. The function checks to see if it is already running,
     * and then crashed. This provides a much more graceful crash with error information.
     */
    Engine::sharedInstance()->runEngine();

    return 0;
}

// ------ cross-platform compatibility ------

#if _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main();
}

#endif /* _WIN32 */

#if __ANDROID__

#include <jni.h>

extern "C" JNIEXPORT jint JNICALL 
Java_com_furred_loader_NativeActivity_androidMain(JNIEnv* env, jobject object) {
    return main();
}

#endif /* __ANDROID__ */

// ------------------------------------------