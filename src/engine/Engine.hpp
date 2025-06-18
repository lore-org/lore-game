#pragma once
#include "Default.hpp"

#include <thread>
#include <atomic>

#include "Scheduler.hpp"
#include "Director.hpp"
#include "presence.hpp"

extern bool rpcIsActive;

inline void SetupEngine() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(720, 480, "lore-game");

    if (presence::isEnabled()) {
        rpcIsActive = false;
        discord::RPCManager::get()
            .setClientID(config.value("DISCORD_CLIENT_ID", "0"))
            .onReady([](auto) {
                rpcIsActive = true;
                fmt::println("Discord Presence initialised");
            })
            .onErrored([](auto, auto) {
                fmt::println("Failed to initialise Discord presence");
            })
            .initialize();
    }
}

constexpr double ticksPerSecond = 240.f;
constexpr double secondsPerTicks = 1.f / ticksPerSecond;

inline double lastTime = GetTime();
inline std::atomic<bool> stopUpdate(false);

inline void RunEngine() {
    std::thread updateThread([]() {
        while (!::stopUpdate) {
            double startTime = GetTime();

            Scheduler::sharedScheduler()->update(startTime - ::lastTime);

            ::lastTime = startTime;

            double endTime = GetTime();

            if (endTime - startTime < ::secondsPerTicks)
                WaitTime((startTime + ::secondsPerTicks) - endTime);
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