#include "SDL3/SDL_render.h"
#include <engine/Default.h>

#include <engine/Engine.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <thread>
#include <atomic>
#ifdef PARALLEL
    #include <execution>
    #define par_unseq std::execution::par_unseq,
#else
    #define par_unseq
#endif
#include <utility>

#include <engine/PresenceManager.h>
#include <engine/Scheduler.h>
#include <engine/Director.h>
#include <engine/config.hpp>
#include <engine/utils.hpp>

#define Millisecond_Constant 1e-3

std::shared_ptr<Engine> Engine::m_instance;

Engine::Engine() :
    m_ticksPerSecond(240.f), m_secondsPerTick(1.f / m_ticksPerSecond),
    m_ticksPerMillisecond(m_ticksPerSecond / 1e3), m_millisecondsPerTick(1.f / m_ticksPerMillisecond),
    m_framesPerSecond(60.f), m_secondsPerFrame(1.f / m_framesPerSecond),
    m_framesPerMillisecond(m_framesPerSecond / 1e3), m_millisecondsPerFrame(1.f / m_framesPerMillisecond),
    m_usingVsync(true),
    m_isStopped(false),
    m_isSetup(false), m_isStarted(false),
    m_windowSize(720, 480),
    m_showFPS(false), m_showTPS(false),
    m_sampleSize(10), m_deltaAverageMult(1.f / m_sampleSize), 
    m_frameAvg(0), m_tickAvg(0),
    m_displayPrecision(0) {};

std::shared_ptr<Engine> Engine::sharedInstance() {
    if (!m_instance) m_instance = utils::protected_make_shared<Engine>();
    return m_instance;
}

double Engine::getTicksPerSecond() {
    return m_ticksPerSecond;
}

void Engine::setTicksPerSecond(double tps) {
    m_ticksPerSecond = tps;
    m_secondsPerTick = 1.f / tps;
    m_ticksPerMillisecond = tps / 1e3;
    m_millisecondsPerTick = 1e3 / tps;
}

void Engine::resetTicksPerSecond() {
    this->setTicksPerSecond(240);
}

double Engine::getSecondsPerTick() {
    return m_secondsPerTick;
}

void Engine::setSecondsPerTick(double spt) {
    this->setTicksPerSecond(1.f / spt);
}

void Engine::resetSecondsPerTick() {
    this->resetTicksPerSecond();
}

double Engine::getFramesPerSecond() {
    return m_framesPerSecond;
};

void Engine::setFramesPerSecond(double fps) {
    if (fps == 0) return void(m_usingVsync = true);
    else m_usingVsync = false;

    m_framesPerSecond = fps;
    m_secondsPerFrame = 1.f / fps;
    m_framesPerMillisecond = fps / 1e3;
    m_millisecondsPerFrame = 1e3 / fps;
};

void Engine::resetFramesPerSecond() {
    this->setFramesPerSecond(0);
};

double Engine::getSecondsPerFrame() {
    return m_secondsPerFrame;
};

void Engine::setSecondsPerFrame(double spf) {
    this->setFramesPerSecond(1.f / spf);
};

void Engine::resetSecondsPerFrame() {
    this->resetFramesPerSecond();
};

void Engine::showFPS(bool show) {
    m_showFPS = show;
}

void Engine::showTPS(bool show) {
    m_showTPS = show;
}

void Engine::setTimeDisplayPrecision(unsigned int precision) {
    m_displayPrecision = precision;
}

void Engine::setTimeDisplaySampleSize(unsigned int size) {
    if (size < 1) throw std::length_error("Sample size must be greater than 0!");
    m_sampleSize = size;
    m_deltaAverageMult = 1.f / m_sampleSize;

    if (m_frameDeltas.capacity() > size) m_frameDeltas.resize(m_sampleSize);
    else m_frameDeltas.resize(m_sampleSize, 0);
    std::ranges::fill(m_frameDeltas, 0);

    if (m_tickDeltas.capacity() > size) m_tickDeltas.resize(m_sampleSize);
    else m_tickDeltas.resize(m_sampleSize, 0);
    std::ranges::fill(m_tickDeltas, 0);
}

void Engine::setWindowSize(Size size) {
    m_windowSize = size;
    if (!SDL_SetWindowSize(m_sdlWindow, size.width, size.height)) PrintSDLError();
}

Size Engine::getStaticWindowSize() {
    return m_windowSize;
}

SDL_Window* Engine::getWindow() {
    return m_sdlWindow;
}

SDL_Renderer* Engine::getRenderer() {
    return m_sdlRenderer;
};

const SDL_DisplayMode* Engine::getDisplayMode() {
    return m_sdlDisplayMode;
};

SDL_Time Engine::getTime() {
    SDL_Time value = {};

    if (!SDL_GetCurrentTime(&value)) PrintSDLError();
    return value;
};

Size Engine::getWindowSize() {
    std::pair<int, int> size(0, 0);
    if (!SDL_GetWindowSize(m_sdlWindow, &size.first, &size.second)) PrintSDLError();

    return MakeSize(size.first, size.second);
};

std::shared_ptr<Engine::MouseData> Engine::getMouseData() {
    MouseData mouseData = {};

    auto mouseState = SDL_GetMouseState(&mouseData.x, &mouseData.y);

    #define ParseButtonMask(x) (mouseState & x) > 0;

    mouseData.lmb = ParseButtonMask(SDL_BUTTON_LMASK);
    mouseData.mmb = ParseButtonMask(SDL_BUTTON_MMASK);
    mouseData.rmb = ParseButtonMask(SDL_BUTTON_RMASK);
    mouseData.side1 = ParseButtonMask(SDL_BUTTON_X1MASK);
    mouseData.side2 = ParseButtonMask(SDL_BUTTON_X2MASK);
    
    return std::make_shared<MouseData>(mouseData);
}

TTF_Font* Engine::getOrCreateFont(std::string file, float point) {
    if (m_fontMap.contains(file)) return m_fontMap.at(file);

    auto font = TTF_OpenFont(file.c_str(), point);
    if (!font) {
        PrintSDLError();
        return nullptr;
    }
    m_fontMap.insert({ file, font });
    return font;
};

void Engine::setupEngine() {
    if (m_isSetup) return fmt::println("Engine has already been set up!");
    m_isSetup = true;

    m_frameDeltas.resize(m_sampleSize, 0);
    m_tickDeltas.resize(m_sampleSize, 0);

    if (!SDL_SetAppMetadata("lore-game", "0.0.1", "dev.kontroll.lore")) PrintSDLError();
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) PrintSDLError();
    if (!TTF_Init()) PrintSDLError();
    if (!SDL_CreateWindowAndRenderer(
        SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING),
        m_windowSize.width, m_windowSize.height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS,
        &m_sdlWindow, &m_sdlRenderer
    )) PrintSDLError();

    this->_updateDisplayData();

    m_sdlTextEngine = TTF_CreateRendererTextEngine(m_sdlRenderer);
    if (!m_sdlTextEngine) PrintSDLError();

    m_fpsText = TTF_CreateText(
        m_sdlTextEngine, this->getOrCreateFont("resources/Noto Sans.ttf"),
        "",
        NULL
    );
    if (!m_fpsText) PrintSDLError();
    m_tpsText = TTF_CreateText(
        m_sdlTextEngine, this->getOrCreateFont("resources/Noto Sans.ttf"),
        "",
        NULL
    );
    if (!m_tpsText) PrintSDLError();

    auto presenceManager = utils::PresenceManager::sharedManager();

    auto clientID = config->at("DISCORD_CLIENT_ID");
    if (!clientID) clientID = "0";

    if (presenceManager->isEnabled()) {
        presenceManager->setActive(false);
        discord::RPCManager::get()
            .setClientID(clientID.get<std::string>())
            .onReady([](auto) {
                utils::PresenceManager::sharedManager()->setActive(true);
                fmt::println("Discord Presence initialised.");
            })
            .onErrored([](auto, auto) {
                fmt::println("Failed to initialise Discord presence.");
            })
            .initialize();
    }
};

void Engine::runEngine() {
    if (!m_isSetup) throw std::runtime_error("Engine not setup before trying to run!");
    if (m_isStarted) return fmt::println("Engine is already running!");
    m_isStarted = true;

    std::thread updateThread([this]() {
        auto scheduler = Scheduler::sharedScheduler();

        double lastTickTime = SDL_GetTicks();
        while (!m_isStopped) {
            double startTime = SDL_GetTicks();

            const double dt = (startTime - lastTickTime) * Millisecond_Constant;
            if (m_showTPS) {
                std::ranges::rotate(m_tickDeltas, m_tickDeltas.begin() + 1);
                m_tickDeltas.back() = 1.f / dt;
            }

            scheduler->update(dt);

            lastTickTime = startTime;

            double endTime = SDL_GetTicks();

            if (endTime - startTime < m_millisecondsPerTick)
                SDL_DelayPrecise(((startTime + m_millisecondsPerTick) - endTime) * 1e6);
        }
    });

    // Specifically for engine-related processes that may take up computational time
    std::thread backgroundThread([this]() {
        while (!m_isStopped) {
            double startTime = SDL_GetTicks();

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
            
            double endTime = SDL_GetTicks();

            if (endTime - startTime < m_millisecondsPerTick)
                SDL_DelayPrecise(((startTime + m_millisecondsPerTick) - endTime) * 1e6);
        }
    });

    
    auto notoSans = this->getOrCreateFont("resources/Noto Sans.ttf");
    auto director = Director::sharedDirector();
    auto format = fmt::format("%.{}f", m_displayPrecision);
    SDL_Event event = {};

    double lastFrameTime = SDL_GetTicks();
    while (!m_isStopped) {
        double startTime = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_MOVED:
                    this->_updateDisplayData();
                    break;
                case SDL_EVENT_QUIT:
                    m_isStopped = true;
                    break;
            }
        }

        const double dt = (startTime - lastFrameTime) * Millisecond_Constant;
        if (m_showFPS) {
            std::ranges::rotate(m_frameDeltas, m_frameDeltas.begin() + 1);
            m_frameDeltas.back() = 1.f / dt;
        }
        director->draw(dt);
        
        if (!TTF_SetFontSize(notoSans, 20)) PrintSDLError();

        if (m_showFPS) {
            if (!TTF_SetTextString(
                m_fpsText,
                fmt::format("{} FPS", fmt::sprintf(format, m_frameAvg.load())).c_str(),
                NULL
            )) PrintSDLError();
        }
        if (m_showTPS) {
            if (!TTF_SetTextString(
                m_tpsText,
                fmt::format("{} TPS", fmt::sprintf(format, m_tickAvg.load())).c_str(),
                NULL
            )) PrintSDLError();
        }

        if (!SDL_SetRenderDrawColor(
            m_sdlRenderer,
            255, 255, 255, 255
        )) PrintSDLError();

        if (m_showFPS && m_fpsText)
            if (!TTF_DrawRendererText(m_fpsText, 5, 5)) PrintSDLError();
        if (m_showTPS && m_tpsText)
            if (!TTF_DrawRendererText(m_tpsText, 5, m_showFPS ? 30 : 5)) PrintSDLError();

        if (!SDL_RenderPresent(m_sdlRenderer)) PrintSDLError();
        
        lastFrameTime = startTime;

        double endTime = SDL_GetTicks();

        if (endTime - startTime < m_millisecondsPerFrame)
            SDL_DelayPrecise(((startTime + m_millisecondsPerFrame) - endTime) * 1e6);
    }

    m_isStopped = true;
    updateThread.join();
    backgroundThread.join();

    SDL_DestroyWindow(m_sdlWindow);
    SDL_DestroyRenderer(m_sdlRenderer);
    TTF_DestroyRendererTextEngine(m_sdlTextEngine);
    std::ranges::for_each(
        m_fontMap,
        [](std::pair<std::string, TTF_Font*> font) {
            TTF_CloseFont(font.second);
        }
    );

    SDL_Quit();
    TTF_Quit();
};

void Engine::_updateDisplayData() {
    if (!m_sdlRenderer) return;
    
    m_sdlDisplay = SDL_GetDisplayForWindow(m_sdlWindow);
    if (!m_sdlDisplay) PrintSDLError();
    m_sdlDisplayMode = SDL_GetCurrentDisplayMode(m_sdlDisplay);
    if (!m_sdlDisplayMode) PrintSDLError();

    m_windowSize = this->getWindowSize();

    this->_updateFrameData();
};

void Engine::_updateFrameData() {
    if (!m_usingVsync) return;

    this->setFramesPerSecond(m_sdlDisplayMode->refresh_rate);
}