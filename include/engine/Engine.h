#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <atomic>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <engine/Geometry.h>

class Engine : public std::enable_shared_from_this<Engine> {
public:
    static std::shared_ptr<Engine> sharedInstance();

    // Gets the target TPS
    double getTicksPerSecond();
    // Sets the target TPS -- default is 240
    void setTicksPerSecond(double tps);
    // Sets TPS to 240
    void resetTicksPerSecond();

    // Gets the target SPT
    double getSecondsPerTick();
    // Sets the target SPF -- default is 1 / 240
    void setSecondsPerTick(double spt);
    // Sets SPT to 1 / 240
    void resetSecondsPerTick();

    // Gets the target FPS
    double getFramesPerSecond();
    // Sets the target FPS -- 0 uses the monitor's target hz, otherwise known as vsync -- default is 0
    void setFramesPerSecond(double fps);
    // Sets FPS to 0
    void resetFramesPerSecond();

    // Gets the target SPF
    double getSecondsPerFrame();
    // Sets the target SPF -- 0 uses the monitor's target hz, otherwise known as vsync -- default is 0
    void setSecondsPerFrame(double spf);
    // Sets SPF to 0
    void resetSecondsPerFrame();

    // Default is false
    void showFPS(bool show);
    // Default is false
    void showTPS(bool show);

    // How many decimal points should follow the FPS/TPS display -- default is 0
    void setTimeDisplayPrecision(unsigned long long precision);
    // How many samples should the FPS/TPS display average -- default is 10
    void setTimeDisplaySampleSize(unsigned long long size);

    void setWindowSize(Size size);
    // Gets the stored window size that is updated after each frame
    //
    // Does not require to be run in the main thread, as it is pulling a value from memory.
    Size getStaticWindowSize();

    struct MouseData {
        bool lmb;
        bool mmb;
        bool rmb;
        bool side1;
        bool side2;
        float x;
        float y;
    };

    SDL_Window* getWindow();
    SDL_Renderer* getRenderer();
    const SDL_DisplayMode* getDisplayMode();

    // Gets the system time in nanoseconds. For a more performant method that doesn't rely on system time, use SDL_GetTicks
    static SDL_Time getTime();
    // It is recommended to use Engine::getStaticWindowSize instead, as it is more performant and is considered fairly accurate
    //
    // Be sure to run this in the main thread
    Size getWindowSize();
    // Be sure to run this in the main thread
    static std::shared_ptr<MouseData> getMouseData();

    TTF_Font* getOrCreateFont(std::string file, double point = 100);

    void setupEngine();

    void runEngine();

protected:
    Engine();

    // ---- Target Updates ----
        
    double m_ticksPerSecond;
    double m_secondsPerTick;
    double m_ticksPerMillisecond;
    double m_millisecondsPerTick;

    double m_framesPerSecond;
    double m_secondsPerFrame;
    double m_framesPerMillisecond;
    double m_millisecondsPerFrame;

    bool m_usingVsync;

    // ------------------------

    // ---- Engine Status ----

    std::atomic<bool> m_isStopped;

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

    std::unordered_map<std::string, TTF_Font*> m_fontMap;

    // ------------------

    // ---- FPS/TPS ----

    bool m_showFPS;
    TTF_Text* m_fpsText;
    bool m_showTPS;
    TTF_Text* m_tpsText;

    unsigned long long m_sampleSize;
    double m_deltaAverageMult;

    std::vector<double> m_frameDeltas;
    std::vector<double> m_tickDeltas;

    std::atomic<double> m_frameAvg;
    std::atomic<double> m_tickAvg;

    unsigned long long m_displayPrecision;

    // -----------------

private:
    static std::shared_ptr<Engine> m_instance;

    void _updateDisplayData();
    void _updateFrameData();
};