#pragma once

#include "glm/ext/matrix_clip_space.hpp"
#include <cmath>
#include <numbers>
#include <string>
#include <algorithm>
#include <vector>
#include <regex>
#include <cctype>

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <glm/mat4x4.hpp>

#include <openssl/sha.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Geometry.h>
#include <engine/Object.h>

#if defined(__GNUC__) || defined(__clang__)
    #define __always_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define __always_inline __forceinline
#else
    #define __always_inline
#endif

#define MakePoint(x, y) Point(static_cast<long double>(x), static_cast<long double>(y))
#define MakeSize(width, height) Size(static_cast<long double>(width), static_cast<long double>(height))
#define MakeRect(x, y, width, height) Rect(static_cast<long double>(x), static_cast<long double>(y), static_cast<long double>(width), static_cast<long double>(height))

#define EPSILON std::numeric_limits<long double>::epsilon()
#define IsZero(f) std::fabs(static_cast<long double>(f)) < EPSILON

#define GetLine() fmt::format("{}:{}", __FILE__, __LINE__).c_str()
#define GetWithTag(tag, x) fmt::format("[{}]: '{}': {}", tag, GetLine(), x)

#define Log(x) SDL_Log(x, 0)
#define LogCritical(x) fmt::println("{}", GetWithTag("CRITICAL", x))
#define LogDebug(x)    fmt::println("{}", GetWithTag("DEBUG",    x))
#define LogError(x)    fmt::println("{}", GetWithTag("ERROR",    x))
#define LogInfo(x)     fmt::println("{}", GetWithTag("INFO",     x))
#define LogTrace(x)    fmt::println("{}", GetWithTag("TRACE",    x))
#define LogVerbose(x)  fmt::println("{}", GetWithTag("VERBOSE",  x))
#define LogWarn(x)     fmt::println("{}", GetWithTag("WARN",     x))

#ifdef FT_CONFIG_OPTION_ERROR_STRINGS
    #define LogFTError(e) LogDebug(fmt::format("FreeType Error: {} (0x{:x})", FT_Error_String(e), e));
#else
    #define LogFTError(e) LogDebug(fmt::format("FreeType Error: 0x{:x}", e));
#endif

#define __Quote__(...) #__VA_ARGS__
#define CreateEventDecl(prefix, event) inline static const char* event = __Quote__(prefix ## __ ## event)

inline std::locale _locale;

namespace utils {
    struct hash_pair final {
        template<class TFirst, class TSecond>
        size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
            uintmax_t hash = std::hash<TFirst>{}(p.first);
            hash <<= sizeof(hash) * 4;
            hash ^= std::hash<TSecond>{}(p.second);
            return std::hash<uintmax_t>{}(hash);
        }
    };

    std::string toLowerCase(std::string str);
    std::string toUpperCase(std::string str);

    template<typename T, typename U, typename F>
    std::vector<T> mapVector(std::vector<U> vec, F replace);

    // Automatically convert from `this` to a shared_ptr of the designated type
    template <class T>
    std::shared_ptr<T> cast_shared(Object* shared);

    template <typename Object>
    std::shared_ptr<Object> protected_make_shared();

    std::vector<std::string> splitString(std::string string, std::regex delimiter = std::regex {"" });

    bool caseInsensitiveCompare(std::string first, std::string second);
    
    long double dot(Point &p1, Point &p2);
    long double dot(Point &p1, Size &p2);
    long double dot(Size &p1, Point &p2);
    long double dot(Size &p1, Size &p2);

    constexpr long double radDivisor = std::numbers::pi / 180.L;

    // Clockwise rotation
    Point rotatePointByCenter(Point& point, Point& center, long double& degrees);

    const char* sha512(const char* input);
    std::string sha512(std::string& input);
    
    struct GLStatus {
        int success;
        char message[2<<9];
    };

    GLStatus checkShaderCompile(GLuint shader);
    GLStatus checkProgramLink(GLuint program);
    GLStatus linkAndUseProgram(GLuint program);

    glm::mat4 createOrthoMat(long double framebufferWidth, long double framebufferHeight);
    glm::mat4 createOrthoMat(Size framebufferSize);
}

template <typename _Size = double>
struct vec2 {
    union {
        struct {
            _Size x;
            _Size y;
        };
        // Not valid in glsl
        struct {
            _Size w;
            _Size h;
        };
        struct {
            _Size r;
            _Size g;
        };
        struct {
            _Size s;
            _Size t;
        };
    };
};

template <typename _Size = double>
struct vec3 {
    union {
        struct {
            _Size x;
            _Size y;
            _Size z;
        };
        // Not valid in glsl
        struct {
            _Size w;
            _Size h;
            _Size l;
        };
        struct {
            _Size r;
            _Size g;
            _Size b;
        };
        struct {
            _Size s;
            _Size t;
            _Size p;
        };
    };
};

template <typename _Size = double>
struct vec4 {
    union {
        struct {
            _Size x;
            _Size y;
            _Size z;
            _Size w;
        };
        struct {
            _Size r;
            _Size g;
            _Size b;
            _Size a;
        };
        struct {
            _Size s;
            _Size t;
            _Size p;
            _Size q;
        };
    };
};