#include <thread>
#include <atomic>

#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "engine/Data.hpp"
#include "engine/Sprite.hpp"
#include "engine/Scheduler.hpp"
#include "engine/Director.hpp"

std::atomic<bool> stopUpdate(false);
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    raylib::InitWindow(720, 480);

    auto spr = Sprite::createFromFile("resources/kitty.png");

    auto updateLoop = Object::createWithUpdate([&spr](float dt) {
        spr->setRotation(spr->getRotation() + (1.f * dt));
        spr->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);
    });
    Scheduler::sharedScheduler()->scheduleUpdate(updateLoop);

    // !SECTION engine code

    constexpr double ticksPerSecond = 240.f;
    constexpr double secondsPerTicks = 1.f / ticksPerSecond;
    std::thread updateThread([]() {
        while (!::stopUpdate) {
            auto startTime = GetTime();

            Scheduler::sharedScheduler()->update(GetFrameTime() * ticksPerSecond);

            auto endTime = GetTime();

            if (endTime - startTime < secondsPerTicks)
                WaitTime((startTime + secondsPerTicks) - endTime);
        }
    });

    while (!WindowShouldClose()) {
        BeginDrawing();
        
        Director::sharedDirector()->draw(GetFrameTime());

        // raylib::DrawText(fmt::format("{} FPS", GetFPS()), 5, 5, 20, WHITE);

        EndDrawing();
    }

    stopUpdate = true;
    updateThread.join();
    CloseWindow();

    // !SECTION
}