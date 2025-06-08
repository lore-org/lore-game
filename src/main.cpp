#include <thread>
#include <atomic>

#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "Data.hpp"
#include "Sprite.hpp"
#include "Scheduler.hpp"

std::atomic<bool> stopUpdate(false);
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    raylib::InitWindow(720, 480);

    auto spr = Sprite::createFromFile("resources/kitty.png");

    int hue = 0;

    auto scheduler = Scheduler::sharedScheduler();

    auto updateLoop = Object::createWithUpdate([&spr](float dt) {
        spr->setRotation(spr->getRotation() + (1.f * dt));
        spr->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);
    });
    scheduler->scheduleUpdate(updateLoop);

    constexpr double ticksPerSecond = 240.f;
    constexpr double secondsPerTicks = 1.f / ticksPerSecond;
    std::thread updateThread([&scheduler]() {
        while (!::stopUpdate) {
            auto startTime = GetTime();

            scheduler->update(GetFrameTime() * ticksPerSecond);

            auto endTime = GetTime();

            if (endTime - startTime < secondsPerTicks)
                WaitTime((startTime + secondsPerTicks) - endTime);
        }
    });

    while (!WindowShouldClose()) {
        if (hue >= 360) hue %= 360;

        BeginDrawing();

        ClearBackground(BLACK);
        raylib::DrawText(fmt::format("{} FPS", GetFPS()), 5, 5, 20, WHITE);

        // TODO - SceneManager to run draw calls
        spr->draw();

        EndDrawing();
    }

    stopUpdate = true;
    updateThread.join();
    CloseWindow();
}