#include <thread>
#include <atomic>

#include <raylib.h>
#include <fmt/base.h>
#include <fmt/format.h>

#include "Scheduler.hpp"
#include "Director.hpp"

inline void SetupEngine() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(720, 480, "lore-game");
}

inline std::atomic<bool> stopUpdate(false);
inline void RunEngine() {
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

        EndDrawing();
    }

    stopUpdate = true;
    updateThread.join();
    CloseWindow();
}