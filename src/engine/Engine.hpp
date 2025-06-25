#pragma once
#include "Default.hpp" // IWYU pragma: keep

#include <stdexcept>
#include <thread>
#include <atomic>
#if PARALLEL
    #include <execution>
    #define par_unseq std::execution::par_unseq,
#else
    #define par_unseq
#endif

#include "Scheduler.hpp"
#include "Director.hpp"
#include "PresenceManager.hpp"

class Engine;

namespace {
    inline Engine* g_engineInstance;
}

class Engine {
public:
    double ticksPerSecond = 240.f;
    double secondsPerTick = 1.f / ticksPerSecond;

    static Engine* sharedInstance() {
        if (!g_engineInstance) g_engineInstance =  new Engine();
        return g_engineInstance;
    }

    // default is 240
    inline void setTicksPerSecond(double tps) {
        ticksPerSecond = tps;
        secondsPerTick = 1.f / ticksPerSecond;
    }
    // sets TPS to 240
    inline void resetTicksPerSecond() {
        this->setTicksPerSecond(240);
    }

    // default is false
    inline void showFPS(bool show) {
        if (show) std::fill(m_frameDeltas.begin(), m_frameDeltas.end(), 0);
        m_showFPS = show;
    }
    // default is false
    inline void showTPS(bool show) {
        if (show) std::fill(m_tickDeltas.begin(), m_tickDeltas.end(), 0);
        m_showTPS = show;
    }

    // how many decimal points should follow the FPS/TPS display -- default is 0
    inline void setTimeDisplayPrecision(unsigned int precision) {
        m_displayPrecision = precision;
    }
    // how many samples should the FPS/TPS display average -- default is 10
    void setTimeDisplaySampleSize(unsigned int size) {
        if (size < 1) throw std::length_error("Sample Size must be greater than 0!");
        m_sampleSize = size;
        m_deltaAverageMult = 1.f / m_sampleSize;

        if (m_frameDeltas.capacity() > size) m_frameDeltas.resize(m_sampleSize);
        else m_frameDeltas.resize(m_sampleSize, 0);
        std::fill(m_frameDeltas.begin(), m_frameDeltas.end(), 0);

        if (m_tickDeltas.capacity() > size) m_tickDeltas.resize(m_sampleSize);
        else m_tickDeltas.resize(m_sampleSize, 0);
        std::fill(m_tickDeltas.begin(), m_tickDeltas.end(), 0);
    }

    // only affects the window before the engine is set up
    void setScreenSize(Size size) {
        if (m_isSetup) return fmt::println("Engine has already been set up!");
        m_screenSize = size;
    }

    void setupEngine() {
        if (m_isSetup) return fmt::println("Engine has already been set up!");
        m_isSetup = true;

        m_frameDeltas.resize(m_sampleSize, 0);
        m_tickDeltas.resize(m_sampleSize, 0);

        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        InitWindow(m_screenSize.width, m_screenSize.height, "lore-game");

        auto presenceManager = utils::PresenceManager::sharedManager();

        if (presenceManager->isEnabled()) {
            presenceManager->setActive(false);
            discord::RPCManager::get()
                .setClientID(config.value("DISCORD_CLIENT_ID", "0"))
                .onReady([](auto) {
                    utils::PresenceManager::sharedManager()->setActive(true);
                    fmt::println("Discord Presence initialised.");
                })
                .onErrored([](auto, auto) {
                    fmt::println("Failed to initialise Discord presence.");
                })
                .initialize();
        }
    }

    void runEngine() {
        if (!m_isSetup) throw std::runtime_error("Engine not setup before trying to run!");
        if (m_isStarted) return fmt::println("Engine is already running!");
        m_isStarted = true;

        m_lastTime = GetTime();
        std::thread updateThread([this]() {
            auto scheduler = Scheduler::sharedScheduler();

            while (!m_stopUpdate) {
                double startTime = GetTime();

                const double dt = startTime - m_lastTime;
                if (m_showTPS) {
                    std::rotate(m_tickDeltas.begin(), m_tickDeltas.begin() + 1, m_tickDeltas.end());
                    m_tickDeltas.back() = 1.f / dt;
                }

                scheduler->update(dt);

                m_lastTime = startTime;

                double endTime = GetTime();

                if (endTime - startTime < secondsPerTick)
                    WaitTime((startTime + secondsPerTick) - endTime);
            }
        });

        // specifically for engine-related processes that may take up computational time
        std::thread backgroundThread([this]() {
            while (!m_stopUpdate) {
                double startTime = GetTime();

                if (m_frameDeltas.size() == 0) return;
                if (m_tickDeltas.size() == 0) return;

                m_frameAvg = m_showFPS ?
                    std::reduce(
                        par_unseq
                        m_frameDeltas.begin(),
                        m_frameDeltas.end()
                    ) / m_sampleSize :
                    0;

                m_tickAvg = m_showTPS ?
                    std::reduce(
                        par_unseq
                        m_tickDeltas.begin(),
                        m_tickDeltas.end()
                    ) / m_sampleSize :
                    0;
                
                double endTime = GetTime();

                if (endTime - startTime < secondsPerTick)
                    WaitTime((startTime + secondsPerTick) - endTime);
            }
        });

        while (!WindowShouldClose()) {
            BeginDrawing();

            const double dt = GetFrameTime();
            if (m_showFPS) {
                std::rotate(m_frameDeltas.begin(), m_frameDeltas.begin() + 1, m_frameDeltas.end());
                m_frameDeltas.back() = 1.f / dt;
            }
            
            Director::sharedDirector()->draw(dt);
            
            auto format = fmt::format("%.{}f", m_displayPrecision);

            if (m_showFPS) DrawText(fmt::format("{} FPS", fmt::sprintf(format, m_frameAvg.load())).c_str(), 5, 5, 20, BLACK);
            if (m_showTPS) DrawText(fmt::format("{} TPS", fmt::sprintf(format, m_tickAvg.load())).c_str(), 5, m_showFPS ? 30 : 5, 20, BLACK);

            EndDrawing();
        }

        m_stopUpdate = true;
        updateThread.join();
        backgroundThread.join();
        CloseWindow();
    }

protected:
    Engine() : m_stopUpdate(false), m_isSetup(false), m_isStarted(false),
        m_showFPS(false), m_showTPS(false), m_displayPrecision(0), m_sampleSize(10),
        m_deltaAverageMult(1.f / m_sampleSize), m_frameAvg(9), m_tickAvg(0),
        m_screenSize(720, 480) {};

    double m_lastTime;
    std::atomic<bool> m_stopUpdate;

    bool m_isSetup;
    bool m_isStarted;

    // ---- FPS/TPS ----

    bool m_showFPS;
    bool m_showTPS;

    unsigned int m_sampleSize;
    double m_deltaAverageMult;

    std::vector<double> m_frameDeltas;
    std::vector<double> m_tickDeltas;

    std::atomic<double> m_frameAvg;
    std::atomic<double> m_tickAvg;

    unsigned int m_displayPrecision;

    // -----------------

    Size m_screenSize;
};