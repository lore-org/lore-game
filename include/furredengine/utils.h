#pragma once

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

#include <ada.h>

#include <httplib.h>

#include <furredengine/config.hpp>
#include <furredengine/Geometry.h>
#include <furredengine/Object.h>

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

namespace FurredEngine::utils {

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
std::vector<T> mapVector(std::vector<U> vec, F replace) {
    std::vector<T> newVec;
    std::ranges::transform(vec, newVec.begin(), replace);
    return newVec;
};

// Automatically convert from `this` to a shared_ptr of the designated type
template <class T>
std::shared_ptr<T> cast_shared(Object* shared) {
    return std::dynamic_pointer_cast<T>(shared->shared_from_this());
};

template <typename Object>
std::shared_ptr<Object> protected_make_shared() {
    struct helper : public Object {};
    return std::make_shared<helper>();
};

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

httplib::Result getURL(std::string url);

}