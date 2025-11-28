#include <engine/Engine.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <utility>
#include <cstdint>

#ifdef HAS_PAR_UNSEQ
    #include <execution>
    #define par_unseq std::execution::par_unseq,
#else
    #define par_unseq
#endif

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <discord-rpc.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>
#include <engine/PresenceManager.h>
#include <engine/Scheduler.h>
#include <engine/Director.h>
#include <engine/Typeable.h>

std::shared_ptr<Engine> Engine::m_instance;

Engine::Engine() :
    m_ticksPerSecond(240.f), m_secondsPerTick(1.f / m_ticksPerSecond),
    m_ticksPerNanosecond(m_ticksPerSecond / NanosecondsPerSecond), m_nanosecondsPerTick(1.f / m_ticksPerNanosecond),
    m_framesPerSecond(60.f), m_secondsPerFrame(1.f / m_framesPerSecond),
    m_framesPerNanosecond(m_framesPerSecond / NanosecondsPerSecond), m_nanosecondsPerFrame(1.f / m_framesPerNanosecond),
    m_usingVsync(true),
    m_isStopped(false),
    m_isSetup(false), m_isStarted(false),
    m_windowSize(720, 480),
    m_showFPS(false), m_showTPS(false),
    m_sampleSize(10), m_deltaAverageMult(1.f / m_sampleSize), 
    m_frameAvg(0), m_tickAvg(0),
    m_displayPrecision(0) {}

std::shared_ptr<Engine> Engine::sharedInstance() {
    if (!m_instance) m_instance = utils::protected_make_shared<Engine>();
    return m_instance;
}

void Engine::setTicksPerSecond(long double tps) {
    m_ticksPerSecond = tps;
    m_secondsPerTick = 1.f / m_ticksPerSecond;
    m_ticksPerNanosecond = tps / NanosecondsPerSecond;
    m_nanosecondsPerTick = 1.f / m_ticksPerNanosecond;
}

void Engine::resetTicksPerSecond() {
    this->setTicksPerSecond(240);
}

void Engine::setSecondsPerTick(long double spt) {
    this->setTicksPerSecond(1.f / spt);
}

void Engine::resetSecondsPerTick() {
    this->resetTicksPerSecond();
}

void Engine::setFramesPerSecond(long double fps) {
    if (fps == 0) return void(m_usingVsync = true);
    else m_usingVsync = false;

    m_framesPerSecond = fps;
    m_secondsPerFrame = 1.f / m_framesPerSecond;
    m_framesPerNanosecond = fps / NanosecondsPerSecond;
    m_nanosecondsPerFrame = 1.f / m_framesPerNanosecond;
};

void Engine::resetFramesPerSecond() {
    this->setFramesPerSecond(0);
}

void Engine::setSecondsPerFrame(long double spf) {
    this->setFramesPerSecond(1.f / spf);
}

void Engine::resetSecondsPerFrame() {
    this->resetFramesPerSecond();
}

void Engine::showFPS(bool show) {
    m_showFPS = show;
}

void Engine::showTPS(bool show) {
    m_showTPS = show;
}

void Engine::setTimeDisplayPrecision(uint64_t precision) {
    m_displayPrecision = precision;
}

void Engine::setTimeDisplaySampleSize(uint64_t size) {
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
    if (!SDL_SetWindowSize(m_sdlWindow, static_cast<int>(size.width), static_cast<int>(size.height))) LogSDLError();
}

SDL_Time Engine::getTime() {
    SDL_Time value = {};

    if (!SDL_GetCurrentTime(&value)) LogSDLError();
    return value;
}

Size Engine::getWindowSize() {
    std::pair<int, int> size(0, 0);
    if (!SDL_GetWindowSize(m_sdlWindow, &size.first, &size.second)) LogSDLError();

    return MakeSize(size.first, size.second);
}

std::shared_ptr<Engine::MouseData> Engine::getMouseData() {
    MouseData mouseData = {};

    auto mouseState = SDL_GetMouseState(&mouseData.x, &mouseData.y);

    #define ParseButtonMask(x) (mouseState & x) > 0

    mouseData.lmb = ParseButtonMask(SDL_BUTTON_LMASK);
    mouseData.mmb = ParseButtonMask(SDL_BUTTON_MMASK);
    mouseData.rmb = ParseButtonMask(SDL_BUTTON_RMASK);
    mouseData.side1 = ParseButtonMask(SDL_BUTTON_X1MASK);
    mouseData.side2 = ParseButtonMask(SDL_BUTTON_X2MASK);
    
    return std::make_shared<MouseData>(mouseData);
}

TTF_Font* Engine::getOrCreateFont(std::string file, float point) {
    std::pair<std::string, float> key(file, point);
    if (m_fontMap.contains(key)) return m_fontMap.at(key);

    auto font = TTF_OpenFont(file.c_str(), point);
    if (!font) {
        LogSDLError();
        return nullptr;
    }
    m_fontMap.insert({ key, font });
    return font;
}

void Engine::requestTextInputCapturing(std::shared_ptr<Typeable> node) {
    for (auto& capture : m_textInputCaptures) {
        if (capture == node) return;
    }

    m_textInputCaptures.push_back(node);
}

void Engine::removeTextInputCapturing(std::shared_ptr<Typeable> node) {
    for (size_t i = 0; i < m_textInputCaptures.size(); i++) {
        auto& capture = m_textInputCaptures.at(i);

        if (capture == node) {
            m_textInputCaptures.erase(m_textInputCaptures.begin() + i);
        }
    }
}

void Engine::setupEngine() {
    if (m_isSetup) return LogWarn("Engine has already been set up!");
    m_isSetup = true;

    m_frameDeltas.resize(m_sampleSize, 0);
    m_tickDeltas.resize(m_sampleSize, 0);

    if (!SDL_SetAppMetadata(__APP_NAME__, __APP_VERSION__, __APP_NAMESPACE__)) LogSDLError();
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) LogSDLError();
    if (!TTF_Init()) LogSDLError();
    if (!SDL_CreateWindowAndRenderer(
        SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING),
        static_cast<int>(m_windowSize.width), static_cast<int>(m_windowSize.height),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS,
        &m_sdlWindow, &m_sdlRenderer
    )) LogSDLError();

    this->_updateDisplayData();

    m_sdlTextEngine = TTF_CreateRendererTextEngine(m_sdlRenderer);
    if (!m_sdlTextEngine) LogSDLError();

    m_fpsText = TTF_CreateText(
        m_sdlTextEngine, this->getOrCreateFont("resources/Noto Sans.ttf"),
        "",
        0
    );
    if (!m_fpsText) LogSDLError();
    m_tpsText = TTF_CreateText(
        m_sdlTextEngine, this->getOrCreateFont("resources/Noto Sans.ttf"),
        "",
        0
    );
    if (!m_tpsText) LogSDLError();

    auto presenceManager = utils::PresenceManager::sharedManager();

    auto clientID = config->at("DISCORD_CLIENT_ID");
    if (!clientID) clientID = "0";

    if (presenceManager->isEnabled()) {
        presenceManager->setActive(false);
        discord::RPCManager::get()
            .setClientID(clientID.get<std::string>())
            .onReady([](auto) {
                utils::PresenceManager::sharedManager()->setActive(true);
                LogInfo("Discord Presence initialised.");
            })
            .onErrored([](auto, auto) {
                LogError("Failed to initialise Discord presence.");
            })
            .initialize();
    }
}

void Engine::runEngine() {
    if (!m_isSetup) throw std::runtime_error("Engine not setup before trying to run!");
    if (m_isStarted) return LogWarn("Engine is already running!");
    m_isStarted = true;

    std::thread updateThread([this]() {
        auto scheduler = Scheduler::sharedScheduler();

        uint64_t lastTickTime = SDL_GetTicksNS();
        while (!m_isStopped) {
            uint64_t startTime = SDL_GetTicksNS();

            const long double dt = (startTime - lastTickTime) * SecondsPerNanosecond;
            if (m_showTPS) {
                std::ranges::rotate(m_tickDeltas, m_tickDeltas.begin() + 1);
                m_tickDeltas.back() = 1.f / dt;
            }

            scheduler->update(dt);

            lastTickTime = startTime;

            uint64_t endTime = SDL_GetTicksNS();

            if (endTime - startTime < m_nanosecondsPerTick)
                SDL_DelayPrecise((startTime + m_nanosecondsPerTick) - endTime);
        }
    });

    // Specifically for engine-related processes that may take up computational time
    std::thread backgroundThread([this]() {
        uint64_t lastUpdateTime = SDL_GetTicksNS();

        while (!m_isStopped) {
            uint64_t startTime = SDL_GetTicksNS();

            if (
                lastUpdateTime + NanosecondsPerSecond < startTime &&
                m_frameDeltas.size() > 0 &&
                m_tickDeltas.size() > 0
            ) {
                lastUpdateTime = startTime;

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
            }
            
            uint64_t endTime = SDL_GetTicksNS();

            if (endTime - startTime < m_nanosecondsPerTick)
                SDL_DelayPrecise((startTime + m_nanosecondsPerTick) - endTime);
        }
    });

    
    auto notoSans = this->getOrCreateFont("resources/Noto Sans.ttf");
    auto director = Director::sharedDirector();
    auto format = fmt::format("%.{}f", m_displayPrecision);
    SDL_Event event = {};

    uint64_t lastFrameTime = SDL_GetTicksNS();
    while (!m_isStopped) {
        uint64_t startTime = SDL_GetTicksNS();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_MOVED:
                    this->_updateDisplayData();
                    break;
                case SDL_EVENT_QUIT:
                case SDL_EVENT_TERMINATING:
                    m_isStopped = true;
                    break;
                case SDL_EVENT_TEXT_INPUT: {
                    auto textInputEvent = *reinterpret_cast<SDL_TextInputEvent*>(&event);
                    std::string text(textInputEvent.text);

                    LogInfo(text);

                    for (auto& node : m_textInputCaptures) {
                        if (!node->isFocused()) return;

                        node->_handleText(text);
                    }
                    
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    auto keyDownEvent = *reinterpret_cast<SDL_KeyboardEvent*>(&event);

                    // TODO - impl mod keys (ctrl, shift, etc.)

                    switch (keyDownEvent.key) {
                        case SDLK_BACKSPACE:
                            LogInfo("SDLK_BACKSPACE");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;

                                node->_handleDelete(Typeable::DeleteType::Backwards);
                            }
                            break;
                        case SDLK_DELETE:
                            LogInfo("SDLK_DELETE");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;

                                node->_handleDelete(Typeable::DeleteType::Forwards);
                            }
                            break;
                        case SDLK_LEFT:
                            LogInfo("SDLK_LEFT");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;

                                auto& seekBounds = node->m_seekBounds;
                                if (seekBounds.start <= 0) continue;

                                node->_handleSeeking(seekBounds.start - 1, 0);
                            }
                            break;
                        case SDLK_RIGHT:
                            LogInfo("SDLK_RIGHT");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;
                                auto inputText = node->m_displayText->getDisplayedText();

                                auto& seekBounds = node->m_seekBounds;
                                if (seekBounds.start >= inputText.size()) continue;

                                node->_handleSeeking(seekBounds.start + 1, 0);
                            }
                            break;
                        case SDLK_END:
                            LogInfo("SDLK_END");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;
                                auto inputText = node->m_displayText->getDisplayedText();

                                node->_handleSeeking(inputText.size(), 0);
                            }
                            break;
                        case SDLK_HOME:
                            LogInfo("SDLK_HOME");

                            for (auto& node : m_textInputCaptures) {
                                if (!node->isFocused()) continue;

                                node->_handleSeeking(0, 0);
                            }
                            break;
                    }
                }
            }
        }

        const long double dt = (startTime - lastFrameTime) * SecondsPerNanosecond;
        if (m_showFPS) {
            std::ranges::rotate(m_frameDeltas, m_frameDeltas.begin() + 1);
            m_frameDeltas.back() = 1.f / dt;
        }
        director->draw(dt);
        
        if (!TTF_SetFontSize(notoSans, 20)) LogSDLError();

        // TODO - use TextNode
        if (m_showFPS) {
            if (!TTF_SetTextString(
                m_fpsText,
                fmt::format("{} FPS", fmt::sprintf(format, m_frameAvg.load())).c_str(),
                0
            )) LogSDLError();
        }
        if (m_showTPS) {
            if (!TTF_SetTextString(
                m_tpsText,
                fmt::format("{} TPS", fmt::sprintf(format, m_tickAvg.load())).c_str(),
                0
            )) LogSDLError();
        }

        if (!TTF_SetTextColor(
            m_fpsText,
            255, 255, 255, 255
        )) LogSDLError();
        if (!TTF_SetTextColor(
            m_tpsText,
            255, 255, 255, 255
        )) LogSDLError();

        if (m_showFPS && m_fpsText)
            if (!TTF_DrawRendererText(m_fpsText, 5, 5)) LogSDLError();
        if (m_showTPS && m_tpsText)
            if (!TTF_DrawRendererText(m_tpsText, 5, m_showFPS ? 30 : 5)) LogSDLError();

        if (!SDL_RenderPresent(m_sdlRenderer)) LogSDLError();
        
        lastFrameTime = startTime;

        uint64_t endTime = SDL_GetTicksNS();

        if (endTime - startTime < m_nanosecondsPerFrame)
            SDL_DelayPrecise((startTime + m_nanosecondsPerFrame) - endTime);
    }

    m_isStopped = true;
    updateThread.join();
    backgroundThread.join();

    SDL_DestroyWindow(m_sdlWindow);
    SDL_DestroyRenderer(m_sdlRenderer);
    TTF_DestroyRendererTextEngine(m_sdlTextEngine);
    std::ranges::for_each(
        m_fontMap,
        [](std::pair<std::pair<std::string, long double>, TTF_Font*> font) {
            TTF_CloseFont(font.second);
        }
    );

    SDL_Quit();
    TTF_Quit();
}

void Engine::_updateDisplayData() {
    if (!m_sdlRenderer) return;
    
    m_sdlDisplay = SDL_GetDisplayForWindow(m_sdlWindow);
    if (!m_sdlDisplay) LogSDLError();
    m_sdlDisplayMode = SDL_GetCurrentDisplayMode(m_sdlDisplay);
    if (!m_sdlDisplayMode) LogSDLError();

    m_windowSize = this->getWindowSize();

    this->_updateFrameData();
}

void Engine::_updateFrameData() {
    if (!m_usingVsync) return;

    this->setFramesPerSecond(m_sdlDisplayMode->refresh_rate);
}