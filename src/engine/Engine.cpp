#include <engine/Engine.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <cstdint>
#include <condition_variable>

#ifdef HAS_PAR_UNSEQ
    #include <execution>
    #define par_unseq std::execution::par_unseq,
#else /* HAS_PAR_UNSEQ */
    #define par_unseq
#endif /* HAS_PAR_UNSEQ */

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Trex/Atlas.hpp>

#include <simdutf.h>

#include <openssl/sha.h>

#include <discord-rpc.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/utils.hpp>
#include <engine/PresenceManager.h>
#include <engine/Scheduler.h>
#include <engine/Director.h>
#include <engine/Typeable.h>
#include <engine/TextNode.h>
#include <engine/Geometry.h>

// TODO - move some functions to their own classes to reduce clutter

std::shared_ptr<Engine> Engine::m_instance;

Engine::Engine() :
    m_ticksPerSecond(240.f), m_secondsPerTick(1.f / m_ticksPerSecond),
    m_ticksPerNanosecond(m_ticksPerSecond / NanosecondsPerSecond),
    m_nanosecondsPerTick(1.f / m_ticksPerNanosecond),
    m_framesPerSecond(60.f), m_secondsPerFrame(1.f / m_framesPerSecond),
    m_framesPerNanosecond(m_framesPerSecond / NanosecondsPerSecond),
    m_nanosecondsPerFrame(1.f / m_framesPerNanosecond),
    m_isSetup(false), m_isStarted(false),
    m_windowSize(720, 480),
    m_nanosecondTimerFrequency(0),
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

void Engine::setFramesPerSecond(long double fps) {
    if (fps == 0) {
        auto videoMode = this->getCurrentVideoMode();
        fps = videoMode->refreshRate;
    }

    m_framesPerSecond = fps;
    m_secondsPerFrame = 1.f / m_framesPerSecond;
    m_framesPerNanosecond = fps / NanosecondsPerSecond;
    m_nanosecondsPerFrame = 1.f / m_framesPerNanosecond;
};

void Engine::showFPS(bool show) {
    if (m_fpsText) m_fpsText->setVisible(show);
    if (m_tpsText) m_tpsText->setPositionY(show ? 30. : 5.);

    if (!show) return;

    std::ranges::fill(m_frameDeltas, m_framesPerSecond);
    m_frameAvg = m_framesPerSecond;
}

void Engine::showTPS(bool show) {
    if (m_tpsText) m_tpsText->setVisible(show);

    if (!show) return;

    std::ranges::fill(m_tickDeltas, m_ticksPerSecond);
    m_tickAvg = m_ticksPerSecond;
}

void Engine::setTimeDisplaySampleSize(uint64_t size) {
    if (size < 1) throw std::length_error("Sample size must be greater than 0!");
    m_sampleSize = size;
    m_deltaAverageMult = 1.f / m_sampleSize;

    if (m_frameDeltas.capacity() > size) m_frameDeltas.resize(m_sampleSize);
    else m_frameDeltas.resize(m_sampleSize);
    std::ranges::fill(m_frameDeltas, m_framesPerSecond);

    if (m_tickDeltas.capacity() > size) m_tickDeltas.resize(m_sampleSize);
    else m_tickDeltas.resize(m_sampleSize);
    std::ranges::fill(m_tickDeltas, m_ticksPerSecond);
}

void Engine::setWindowSize(Size size) {
    glfwSetWindowSize(m_glWindow, static_cast<int>(size.width), static_cast<int>(size.height));
    m_windowSize = size;
}

GLFWmonitor* Engine::getCurrentMonitor() {
    Point windowPos;
    glfwGetWindowPos(
        m_glWindow,
        reinterpret_cast<int*>(&windowPos.x),
        reinterpret_cast<int*>(&windowPos.x)
    );

    Point windowCenter {
        windowPos.x + this->getWindowWidth() / 2,
        windowPos.y + this->getWindowHeight() / 2
    };

    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    GLFWmonitor* currentMonitor = nullptr;

    for (int i = 0; i < monitorCount; i++) {
        GLFWmonitor* monitor = *(monitors + i);

        auto videoMode = glfwGetVideoMode(monitor);

        int monitorX, monitorY;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);

        Rect monitorRect {
            static_cast<long double>(monitorX),
            static_cast<long double>(monitorY),
            static_cast<long double>(videoMode->width),
            static_cast<long double>(videoMode->height)
        };

        if (monitorRect.containsPoint(windowCenter)) {
            currentMonitor = monitor;
            break;
        }
    }

    return currentMonitor;
}

Size Engine::getMonitorDPI(GLFWmonitor* monitor) {
    int monitorWidthMM, monitorHeightMM;
    glfwGetMonitorPhysicalSize(
        monitor,
        &monitorWidthMM, &monitorHeightMM
    );

    auto videoMode = glfwGetVideoMode(monitor);

    float monitorWidthIn = monitorWidthMM / 25.4f;
    float monitorHeightIn = monitorHeightMM / 25.4f;

    float horizontalDPI = videoMode->width / monitorWidthIn;
    float verticalDPI = videoMode->height / monitorHeightIn;

    return MakeSize(horizontalDPI, verticalDPI);
}

Trex::Atlas* Engine::getFontAtlas(std::string file, float point) {
    FontAtlasDict key(file, point);
    if (m_fontAtlasMap.contains(key)) return m_fontAtlasMap.at(key);

    return nullptr;
}

Engine::FontAtlasDict Engine::getFontAtlas(Trex::Atlas* fontAtlas) {
    for (auto& storedFontAtlas : m_fontAtlasMap) {
        if (storedFontAtlas.second == fontAtlas) return storedFontAtlas.first;
    }

    return { "", 0 };
}

Trex::Atlas* Engine::createFont(std::string file, float point) {
    auto fontAtlas = new Trex::Atlas(
        file, point
    );

    FontAtlasDict key(file, point);
    if (!m_fontAtlasMap.contains(key)) {
        m_fontAtlasMap.insert({ key, fontAtlas });
    }

    return fontAtlas;
}

Trex::Atlas* Engine::getOrCreateFontAtlas(std::string file, float point) {
    if (auto fontAtlas = this->getFontAtlas(file, point)) return fontAtlas;
    return this->createFont(file, point);
}

Engine::Shader Engine::loadShaderFromFile(ShaderType type, std::string file, const char* tag) {
    for (auto& shader : m_loadedShaders) {
        if (shader.tag == tag) return shader;
    }

    std::ifstream shaderFile(file, std::ios::binary | std::ios::ate);
    if (!shaderFile) {
        LogError(fmt::format("Could not open shader file '{}.'", file));
        return { 0, ShaderType::Unknown };
    }

    size_t size = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);

    std::string shaderSource(size, '\0');
    shaderFile.read(&shaderSource[0], size);

    shaderFile.close();

    return this->loadShaderFromSource(type, shaderSource);
}

Engine::Shader Engine::loadShaderFromSource(ShaderType type, std::string source, const char* tag) {
    if (tag == NULL) tag = utils::sha512(source.c_str());
    
    for (auto& shader : m_loadedShaders) {
        if (shader.tag == tag) return shader;
    }

    auto shaderSource = source.c_str();
    auto shader = glCreateShader(static_cast<GLenum>(type));
    glShaderSource(shader, sizeof(*tag), &shaderSource, NULL);
    glCompileShader(shader);

    auto status = utils::checkShaderCompile(shader);
    if (!status.success) {
        LogError(status.message);
        return { 0, ShaderType::Unknown };
    }

    Shader ret { shader, type, tag };
    m_loadedShaders.push_back(ret);

    return ret;
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

void Engine::requestFramebufferUpdates(GLuint glProgram) {
    glUseProgram(glProgram);
    glUniformMatrix4fv(
        glGetUniformLocation(glProgram, "orthoMat"),
        1, GL_FALSE,
        glm::value_ptr(utils::createOrthoMat(this->getFrameBufferSize()))
    );

    for (auto& program : m_framebufferUpdates) {
        if (program == glProgram) return;
    }

    m_framebufferUpdates.push_back(glProgram);
}

void Engine::removeFramebufferUpdates(GLuint glProgram) {
    for (size_t i = 0; i < m_framebufferUpdates.size(); i++) {
        auto& program = m_framebufferUpdates.at(i);

        if (program == glProgram) {
            m_framebufferUpdates.erase(m_framebufferUpdates.begin() + i);
            break;
        }
    }
}

void Engine::setupEngine() {
    if (m_isSetup) return LogWarn("Engine has already been set up.");

    m_frameDeltas.resize(m_sampleSize, m_framesPerSecond);
    m_tickDeltas.resize(m_sampleSize, m_ticksPerSecond);

    glfwInit();

    // Set after init of glfw
    m_nanosecondTimerFrequency = NanosecondsPerSecond / glfwGetTimerFrequency();

    // OpenGL 3.2 works with android and supports modern features.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // TODO - test supporting 4.1 while keeping android support
// #if __ANDROID__
//     // OpenGL 3.2 works with android and supports modern features.
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
// #else
//     // OpenGL 4.1 is minimum version MacOS Metal supports .
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
// #endif

    // Good practice, and required for MacOS.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

#if __ANDROID__
    // Android requires OpenGL ES.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif /* __ANDROID__ */

    glfwWindowHint(GLFW_SAMPLES, static_cast<int>(MSAALevel::x4));

    // Debugging context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    m_glWindow = glfwCreateWindow(
        static_cast<int>(m_windowSize.width), static_cast<int>(m_windowSize.height),
        __APP_NAME__,
        nullptr, nullptr
    );
    if (!m_glWindow) return LogError("Could not create GLFW Window.");

    glfwMakeContextCurrent(m_glWindow);
    if (gladLoadGL(glfwGetProcAddress) == 0) return LogError("Could not initialise OpenGL with GLAD.");
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(m_glWindow, &_glfwWindowSizeCallback);
    glfwSetFramebufferSizeCallback(m_glWindow, &_glfwFramebufferSizeCallback);
    glfwSetCharCallback(m_glWindow, &_glfwCharCallback);
    glfwSetKeyCallback(m_glWindow, &_glfwKeyCallback);
    glfwSetCursorPosCallback(m_glWindow, &_glfwCursorPosCallback);
    glfwSetMouseButtonCallback(m_glWindow, &_glfwMouseButtonCallback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // debug
    // glDisable(GL_BLEND);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int winWidth, winHeight;
    glfwGetWindowSize(m_glWindow, &winWidth, &winHeight);
    this->_glfwWindowSizeCallback(m_glWindow, winWidth, winHeight);

    int fbufWidth, fbufHeight;
    glfwGetFramebufferSize(m_glWindow, &fbufWidth, &fbufHeight);
    this->_glfwFramebufferSizeCallback(m_glWindow, fbufWidth, fbufHeight);


    m_fpsText = TextNode::create(
        nullptr, 20,
        { 5, 5 }
    );
    m_fpsText->setAnchorPoint(0);

    m_tpsText = TextNode::create(
        nullptr, 20,
        { 5, m_fpsText->isVisible() ? 30 : 5. }
    );
    m_tpsText->setAnchorPoint(0);


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
                LogError("Discord Presence quit unexpectedly.");
            })
            .initialize();
    }

    m_isSetup = true;
}

void Engine::runEngine() {
    if (!m_isSetup) throw std::runtime_error("Engine not setup before trying to run!");
    if (m_isStarted) return LogWarn("Engine is already running.");

    std::thread updateThread([this]() {
        auto scheduler = Scheduler::sharedScheduler();

        auto lastTickTime = Engine::getTimeNS();
        while (!glfwWindowShouldClose(m_glWindow)) {
            auto startTime = Engine::getTimeNS();

            const long double dt = (startTime - lastTickTime) * SecondsPerNanosecond;
            if (m_tpsText->isVisible()) {
                std::ranges::rotate(m_tickDeltas, m_tickDeltas.begin() + 1);
                m_tickDeltas.back() = 1.f / dt;
            }

            scheduler->update(dt);

            lastTickTime = startTime;

            auto endTime = Engine::getTimeNS();
            Engine::preciseNanosecondDelay(startTime, endTime, m_nanosecondsPerTick);
        }
    });

    // Specifically for engine-related processes that may take up computational time
    std::thread backgroundThread([this]() {
        uint64_t lastUpdateTime = Engine::getTimeNS();

        while (!glfwWindowShouldClose(m_glWindow)) {
            uint64_t startTime = Engine::getTimeNS();

            if (
                lastUpdateTime + NanosecondsPerSecond < startTime &&
                m_frameDeltas.size() > 0 &&
                m_tickDeltas.size() > 0
            ) {
                lastUpdateTime = startTime;

                m_frameAvg = std::reduce(
                        par_unseq
                        m_frameDeltas.begin(),
                        m_frameDeltas.end()
                    ) * m_deltaAverageMult;

                m_tickAvg = std::reduce(
                        par_unseq
                        m_tickDeltas.begin(),
                        m_tickDeltas.end()
                    ) * m_deltaAverageMult;
            }
            
            uint64_t endTime = Engine::getTimeNS();
            Engine::preciseNanosecondDelay(startTime, endTime, m_nanosecondsPerTick);
        }
    });

    auto director = Director::sharedDirector();
    auto format = fmt::format("%.{}f", m_displayPrecision);

    glfwShowWindow(m_glWindow);
    
    m_isStarted = true;

    std::mutex mtx;
    std::condition_variable cv;

    auto lastFrameTime = Engine::getTimeNS();
    while (!glfwWindowShouldClose(m_glWindow)) {
        auto startTime = Engine::getTimeNS();

        glfwPollEvents();

        const long double dt = (startTime - lastFrameTime) * SecondsPerNanosecond;
        if (m_fpsText->isVisible()) {
            std::ranges::rotate(m_frameDeltas, m_frameDeltas.begin() + 1);
            m_frameDeltas.back() = 1.f / dt;
        }

        if (m_fpsText->isVisible()) m_fpsText->setDisplayedText(fmt::format("{} FPS", fmt::sprintf(format, m_frameAvg)));
        if (m_tpsText->isVisible()) m_tpsText->setDisplayedText(fmt::format("{} TPS", fmt::sprintf(format, m_tickAvg)));

        director->draw(dt);
        m_fpsText->draw(dt);
        m_tpsText->draw(dt);
        
        lastFrameTime = startTime;

        // auto endTime = Engine::getTimeNS();

        glfwSwapBuffers(m_glWindow);
    }

    updateThread.join();
    backgroundThread.join();

    std::ranges::for_each(
        m_fontAtlasMap,
        [](decltype(m_fontAtlasMap)::value_type fontAtlas) {
            delete fontAtlas.second;
        }
    );

    glfwDestroyWindow(m_glWindow);
    glfwTerminate();
}

void Engine::_glfwWindowSizeCallback(GLFWwindow*, int width, int height) {
    m_instance->m_windowSize = MakeSize(width, height);
    // LogDebug(fmt::format("m_windowSize: {}", m_instance->m_windowSize));
}

void Engine::_glfwFramebufferSizeCallback(GLFWwindow*, int width, int height) {
    m_instance->m_framebufferSize = MakeSize(width, height);
    // LogDebug(fmt::format("m_framebufferSize: {}", m_instance->m_framebufferSize));

    glViewport(0, 0, width, height);

    for (auto& program : m_instance->m_framebufferUpdates) {
        glUseProgram(program);
        glUniformMatrix4fv(
            glGetUniformLocation(program, "orthoMat"),
            1, GL_FALSE,
            glm::value_ptr(utils::createOrthoMat(width, height))
        );
    }
}

void Engine::_glfwCharCallback(GLFWwindow*, unsigned int codepoint) {
    std::u32string text { static_cast<char32_t>(codepoint) };
    std::string result(simdutf::utf8_length_from_utf32(text), '\0');

    if (!simdutf::convert_utf32_to_utf8(
        text.data(), result.size(),
        result.data()
    )) return;

    for (auto& node : Engine::sharedInstance()->m_textInputCaptures) {
        if (!node->isFocused()) continue;

        node->_handleText(result);
    }
}

void Engine::_glfwKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // TODO - impl modifier keys
    // TODO - impl copy and paste w/ glfwGetClipboardString

    switch(key) {
        case GLFW_KEY_BACKSPACE:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;

                node->_handleDelete(Typeable::DeleteType::Backwards);
            }
            break;
        case GLFW_KEY_DELETE:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;

                node->_handleDelete(Typeable::DeleteType::Forwards);
            }
            break;
        case GLFW_KEY_LEFT:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;

                auto& seekBounds = node->m_seekBounds;
                if (seekBounds.start <= 0) continue;

                node->_handleSeeking(seekBounds.start - 1, 0);
            }
            break;
        case GLFW_KEY_RIGHT:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;
                auto inputText = node->m_displayText->getDisplayedText();

                auto& seekBounds = node->m_seekBounds;
                if (seekBounds.start >= inputText.size()) continue;

                node->_handleSeeking(seekBounds.start + 1, 0);
            }
            break;
        case GLFW_KEY_END:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;
                auto inputText = node->m_displayText->getDisplayedText();

                node->_handleSeeking(inputText.size(), 0);
            }
            break;
        case GLFW_KEY_HOME:
            for (auto& node : m_instance->m_textInputCaptures) {
                if (!node->isFocused()) continue;

                node->_handleSeeking(0, 0);
            }
            break;
    }
}

void Engine::_glfwCursorPosCallback(GLFWwindow*, double x, double y) {
    m_instance->m_mouseState.x = x;
    m_instance->m_mouseState.y = y;
}

void Engine::_glfwMouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
    #define CheckButtonAndAssign(mouseButton, button) if (button) m_instance->m_mouseState.mouseButton = action == GLFW_PRESS;

    CheckButtonAndAssign(lmb, GLFW_MOUSE_BUTTON_LEFT);
    CheckButtonAndAssign(mmb, GLFW_MOUSE_BUTTON_MIDDLE);
    CheckButtonAndAssign(rmb, GLFW_MOUSE_BUTTON_RIGHT);
    CheckButtonAndAssign(side1, GLFW_MOUSE_BUTTON_4); // Commonly the Forward button
    CheckButtonAndAssign(side2, GLFW_MOUSE_BUTTON_5); // Commonly the Backward button
}