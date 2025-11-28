#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>

#include <engine/Geometry.h>
#include <engine/utils.hpp>

class Typeable;

#define NanosecondsPerSecond 1e9L
#define SecondsPerNanosecond 1e-9L

class Engine : public std::enable_shared_from_this<Engine> {
public:
    static std::shared_ptr<Engine> sharedInstance();

    // Gets the target TPS
    inline long double getTicksPerSecond() { return m_ticksPerSecond; }
    // Sets the target TPS -- default is 240
    void setTicksPerSecond(long double tps);
    // Sets TPS to 240
    void resetTicksPerSecond();

    // Gets the target SPT
    inline long double getSecondsPerTick() { return m_secondsPerTick; }
    // Sets the target SPF -- default is 1 / 240
    void setSecondsPerTick(long double spt);
    // Sets SPT to 1 / 240
    void resetSecondsPerTick();

    // Gets the target FPS
    inline long double getFramesPerSecond() { return m_framesPerSecond; }
    // Sets the target FPS -- 0 uses the monitor's target hz, otherwise known as vsync -- default is 0
    void setFramesPerSecond(long double fps);
    // Sets FPS to 0
    void resetFramesPerSecond();

    // Gets the target SPF
    inline long double getSecondsPerFrame() { return m_secondsPerFrame; }
    // Sets the target SPF -- 0 uses the monitor's target hz, otherwise known as vsync -- default is 0
    void setSecondsPerFrame(long double spf);
    // Sets SPF to 0
    void resetSecondsPerFrame();

    // Default is false
    void showFPS(bool show);
    // Default is false
    void showTPS(bool show);

    // How many decimal points should follow the FPS/TPS display -- default is 0
    void setTimeDisplayPrecision(uint64_t precision);
    // How many samples should the FPS/TPS display average -- default is 10
    void setTimeDisplaySampleSize(uint64_t size);

    void setWindowSize(Size size);
    // Gets the stored window size that is updated after each frame
    //
    // Does not require to be run in the main thread, as it is pulling a value from memory.
    inline Size getStaticWindowSize() { return m_windowSize; }

    struct MouseData {
        bool lmb;
        bool mmb;
        bool rmb;
        bool side1;
        bool side2;
        float x;
        float y;
    };

    inline SDL_Window* getWindow() { return m_sdlWindow; }
    inline SDL_Renderer* getRenderer() { return m_sdlRenderer; }
    inline const SDL_DisplayMode* getDisplayMode() { return m_sdlDisplayMode; }
    inline TTF_TextEngine* getTextEngine() { return m_sdlTextEngine; }

    // Gets the system time in nanoseconds. For a more performant method that doesn't rely on system time, use SDL_GetTicks
    static SDL_Time getTime();
    // It is recommended to use Engine::getStaticWindowSize instead, as it is more performant and is considered fairly accurate
    //
    // Be sure to run this in the main thread
    Size getWindowSize();
    // Be sure to run this in the main thread
    static std::shared_ptr<MouseData> getMouseData();

    TTF_Font* getOrCreateFont(std::string file, float point = 100.f);

    void requestTextInputCapturing(std::shared_ptr<Typeable> node);
    void removeTextInputCapturing(std::shared_ptr<Typeable> node);

    void setupEngine();

    void runEngine();

protected:
    Engine();

    // ---- Target Updates ----

    long double m_ticksPerSecond;
    long double m_secondsPerTick;
    long double m_ticksPerNanosecond;
    long double m_nanosecondsPerTick;

    long double m_framesPerSecond;
    long double m_secondsPerFrame;
    long double m_framesPerNanosecond;
    long double m_nanosecondsPerFrame;

    bool m_usingVsync;

    // ------------------------

    // ---- Engine Status ----

    bool m_isStopped;

    bool m_isSetup;
    bool m_isStarted;

    Size m_windowSize;

    // -----------------------

    // ---- SDL data ----

    SDL_Window* m_sdlWindow;
    SDL_Renderer* m_sdlRenderer;
    SDL_DisplayID m_sdlDisplay;
    const SDL_DisplayMode* m_sdlDisplayMode;
    TTF_TextEngine* m_sdlTextEngine;

    std::unordered_map<std::pair<std::string, float>, TTF_Font*, utils::hash_pair> m_fontMap;

    std::vector<std::shared_ptr<Typeable>> m_textInputCaptures;

    // ------------------

    // ---- FPS/TPS ----

    bool m_showFPS;
    TTF_Text* m_fpsText;
    bool m_showTPS;
    TTF_Text* m_tpsText;

    uint64_t m_sampleSize;
    long double m_deltaAverageMult;

    std::vector<long double> m_frameDeltas;
    std::vector<long double> m_tickDeltas;

    long double m_frameAvg;
    long double m_tickAvg;

    uint64_t m_displayPrecision;

    // -----------------

private:
    static std::shared_ptr<Engine> m_instance;

    void _updateDisplayData();
    void _updateFrameData();
};