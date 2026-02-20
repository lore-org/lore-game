#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <thread>

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <Trex/Atlas.hpp>

#include <Trex/Atlas.hpp>

#include <engine/Geometry.h>
#include <engine/utils.h>

class TextNode;
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
    inline void resetTicksPerSecond() { this->setTicksPerSecond(240); };

    // Gets the target SPT
    inline long double getSecondsPerTick() { return m_secondsPerTick; }
    // Sets the target SPF -- default is 1 / 240
    inline void setSecondsPerTick(long double spt) { this->setTicksPerSecond(1.f / spt); };
    // Sets SPT to 1 / 240
    inline void resetSecondsPerTick() { this->resetTicksPerSecond(); };

    // Gets the target FPS
    inline long double getFramesPerSecond() { return m_framesPerSecond; }
    // Sets the target FPS -- 0 uses the monitor's target hz -- default is 60
    void setFramesPerSecond(long double fps);
    // Sets FPS to 60
    inline void resetFramesPerSecond() { this->setFramesPerSecond(60); };

    // Gets the target SPF
    inline long double getSecondsPerFrame() { return m_secondsPerFrame; }
    // Sets the target SPF -- 0 uses the monitor's target hz -- default is 1 / 60
    inline void setSecondsPerFrame(long double spf) { this->setFramesPerSecond(1.f / spf); };
    // Sets SPF to 1 / 60
    inline void resetSecondsPerFrame() { this->resetFramesPerSecond(); };

    // Default is false
    void showFPS(bool show);
    // Default is false
    void showTPS(bool show);

    // How many decimal points should follow the FPS/TPS display -- default is 0
    inline void setTimeDisplayPrecision(uint64_t precision) { m_displayPrecision = precision; };
    // How many samples should the FPS/TPS display average -- default is 10
    void setTimeDisplaySampleSize(uint64_t size);

    // Uses spinlock with <= 100 nanosecond busy wait
    __always_inline
    static inline void preciseNanosecondDelay(uint64_t startTime, uint64_t endTime, uint64_t maximumDuration) {
        auto spentDuration = endTime - startTime;
        if (spentDuration < maximumDuration) {
            preciseNanosecondDelay(maximumDuration - spentDuration);
        }
    }

    // Uses spinlock with <= 100 nanosecond busy wait
    __always_inline
    static inline void preciseNanosecondDelay(uint64_t spentDuration, uint64_t maximumDuration) {
        if (spentDuration < maximumDuration) {
            preciseNanosecondDelay(maximumDuration - spentDuration);
        }
    }

    // Uses spinlock with <= 100 nanosecond busy wait
    __always_inline
    static inline void preciseNanosecondDelay(uint64_t duration) {
        auto startTime = Engine::getTimeNS();

        if (std::chrono::nanoseconds(duration) > std::chrono::nanoseconds(100)) {
            std::this_thread::sleep_for(
                std::chrono::nanoseconds(duration) -
                std::chrono::nanoseconds(100)
            );
        }

        // busy waiting, should only take <= 100 nanoseconds of cpu time
        while (Engine::getTimeNS() - startTime <= duration) {}
    }

    void setWindowSize(Size size);
    inline void setWindowWidth(long double width) { setWindowSize({ width, getWindowHeight() }); };
    inline void setWindowHeight(long double height) { setWindowSize({ getWindowWidth(), height }); };

    inline Size getWindowSize() { return m_windowSize; };
    inline long double getWindowWidth() { return m_windowSize.width; };
    inline long double getWindowHeight() { return m_windowSize.height; };
    
    inline Size getFrameBufferSize() { return m_framebufferSize; };
    inline long double getFrameBufferWidth() { return m_framebufferSize.width; };
    inline long double getFrameBufferHeight() { return m_framebufferSize.height; };

    struct MouseData {
        bool lmb = false;
        bool mmb = false;
        bool rmb = false;
        bool side1 = false;
        bool side2 = false;
        double x = 0;
        double y = 0;
    };

    enum class MSAALevel : int {
        Off = 0,
        x2 = 2,
        x4 = 4,
        x8 = 8,
        x16 = 16
    };

    inline GLFWwindow* getWindow() { return m_glWindow; }

    // Gets the time in seconds since initialisation.
    inline static double getTime() { return glfwGetTime(); };
    // Gets the time in nanoseconds since initialisation.
    inline static uint64_t getTimeNS() { return glfwGetTimerValue() * m_instance->m_nanosecondTimerFrequency; };
    // Gets the current state of the mouse since the last event polling.
    inline MouseData getMouseData() { return m_mouseState; };
    // Gets the current monitor that the program is located on.
    GLFWmonitor* getCurrentMonitor();
    // Gets the current video mode of the monitor that the program is located on.
    inline const GLFWvidmode* getCurrentVideoMode() { return glfwGetVideoMode(this->getCurrentMonitor()); };
    // Gets the current DPI of any valid monitor.
    Size getMonitorDPI(GLFWmonitor* monitor);

    // Mapped to <file, point>
    using FontAtlasDict = std::pair<std::string, float>;

    Trex::Atlas* getFontAtlas(std::string file, float point = 100);
    FontAtlasDict getFontAtlas(Trex::Atlas* fontAtlas);
    Trex::Atlas* createFont(std::string file, float point = 100);
    Trex::Atlas* getOrCreateFontAtlas(std::string file, float point = 100);

    enum class ShaderType : GLenum {
        Unknown = 0,
        Compute = GL_COMPUTE_SHADER,
        Vertex = GL_VERTEX_SHADER,
        TesselationControl = GL_TESS_CONTROL_SHADER,
        TesselationEvaluation = GL_TESS_EVALUATION_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Fragment = GL_FRAGMENT_SHADER
    };

    typedef GLuint ShaderID;

    struct Shader {
        ShaderID shaderID;
        ShaderType shaderType;
        const char* tag;
    };
    
    Shader loadShaderFromFile(ShaderType type, std::string file, const char* tag = NULL);
    Shader loadShaderFromSource(ShaderType type, std::string source, const char* tag = NULL);

    void requestTextInputCapturing(std::shared_ptr<Typeable> node);
    void removeTextInputCapturing(std::shared_ptr<Typeable> node);

    // sets gl vec2 uniform `viewportSize` every time the viewport size is updated
    void requestFramebufferUpdates(GLuint glProgram);
    void removeFramebufferUpdates(GLuint glProgram);

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

    // ------------------------

    // ---- Engine Status ----

    bool m_isSetup;
    bool m_isStarted;

    Size m_windowSize;
    Size m_framebufferSize;

    MouseData m_mouseState;

    long double m_nanosecondTimerFrequency;

    // -----------------------

    // ---- Engine data ----

    GLFWwindow* m_glWindow;

    std::unordered_map<FontAtlasDict, Trex::Atlas*, utils::hash_pair> m_fontAtlasMap;

    std::vector<Shader> m_loadedShaders;
    std::vector<std::shared_ptr<Typeable>> m_textInputCaptures;
    std::vector<GLuint> m_framebufferUpdates;

    // ------------------

    // ---- FPS/TPS ----

    std::shared_ptr<TextNode> m_fpsText;
    std::shared_ptr<TextNode> m_tpsText;

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

    static void _glfwWindowSizeCallback(GLFWwindow*, int width, int height);
    static void _glfwFramebufferSizeCallback(GLFWwindow*, int width, int height);
    static void _glfwCharCallback(GLFWwindow*, unsigned int codepoint);
    static void _glfwKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
    static void _glfwCursorPosCallback(GLFWwindow*, double x, double y);
    static void _glfwMouseButtonCallback(GLFWwindow*, int button, int action, int mods);
};