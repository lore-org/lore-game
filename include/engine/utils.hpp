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

#include <openssl/sha.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Geometry.h>
#include <engine/Object.h>

// TODO - make this a .h file with a corresponding .cpp file

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
    inline std::string toLowerCase(std::string str) {
        std::ranges::transform(
            str, str.begin(),
            [&](char ch) { return std::use_facet<std::ctype<char>>(::_locale).tolower(ch); }
        );
        return str;
    }

    inline std::string toUpperCase(std::string str) {
        std::ranges::transform(
            str, str.begin(),
            [&](char ch) { return std::use_facet<std::ctype<char>>(::_locale).toupper(ch); }
        );
        return str;
    }

    template<typename T, typename U, typename F>
    inline std::vector<T> mapVector(std::vector<U> vec, F replace) {
        std::vector<T> newVec;
        std::ranges::transform(vec, newVec.begin(), replace);
        return newVec;
    }

    // Automatically convert from `this` to a shared_ptr of the designated type
    template <class T>
    inline std::shared_ptr<T> cast_shared(Object* shared) {
        return std::dynamic_pointer_cast<T>(shared->shared_from_this());
    }

    template <typename Object>
    inline std::shared_ptr<Object> protected_make_shared() {
        struct helper : public Object {};

        return std::make_shared<helper>();
    }    

    inline std::vector<std::string> splitString(std::string string, std::regex delimiter = std::regex {""}) {
        std::vector<std::string> tokens;
        std::sregex_token_iterator iter(string.begin(), string.end(), delimiter, -1);
        std::sregex_token_iterator end;
        while (end != iter) {
            tokens.push_back(*iter++);
        }
        return tokens;
    }

    inline bool caseInsensitiveCompare(std::string first, std::string second) {
        return std::ranges::equal(first, second, [](char a, char b) { return tolower(a) == tolower(b); });
    }

    struct hash_pair final {
        template<class TFirst, class TSecond>
        size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
            uintmax_t hash = std::hash<TFirst>{}(p.first);
            hash <<= sizeof(hash) * 4;
            hash ^= std::hash<TSecond>{}(p.second);
            return std::hash<uintmax_t>{}(hash);
        }
    };
    
    inline long double dot(Point &p1, Point &p2) { return (p1.x * p2.x) + (p1.y * p2.y); };
    inline long double dot(Point &p1, Size &p2) { return (p1.x * p2.width) + (p1.y * p2.height); };
    inline long double dot(Size &p1, Point &p2) { return (p1.width * p2.x) + (p1.height * p2.y); };
    inline long double dot(Size &p1, Size &p2) { return (p1.width * p2.width) + (p1.height * p2.height); };

    constexpr long double radDivisor = std::numbers::pi / 180.L;

    // Clockwise rotation
    inline Point rotatePointByCenter(Point& point, Point& center, long double& degrees) {
        auto radians = -degrees * radDivisor;

        auto cos = std::cos(radians);
        auto sin = std::sin(radians);

        auto translatedX = point.x - center.x;
        auto translatedY = point.y - center.y;

        return {
            (translatedX * cos - translatedY * sin) + center.x,
            (translatedX * sin + translatedY * cos) + center.y
        };
    }

    inline const char* sha512(const char* input) {
        unsigned char hash[SHA512_DIGEST_LENGTH];

        SHA512(
            reinterpret_cast<const unsigned char*>(input),
            strlen(input),
            hash
        );

        auto out = reinterpret_cast<char*>(calloc(SHA512_DIGEST_LENGTH + 1, sizeof(char)));
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(out + (i * 2), "%02x", hash[i]);
        }

        return const_cast<const char*>(out);
    }

    inline std::string sha512(std::string& input) {
        return sha512(input.c_str());
    }
    
    struct GLStatus {
        int success;
        char message[2<<9];
    };

    inline GLStatus checkShaderCompile(GLuint shader) {
        GLStatus status;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &status.success);
        glGetShaderInfoLog(shader, sizeof(status.message), NULL, status.message);

        return status;
    }

    inline GLStatus checkProgramLink(GLuint program) {
        GLStatus status;

        glGetProgramiv(program, GL_LINK_STATUS, &status.success);
        glGetProgramInfoLog(program, sizeof(status.message), NULL, status.message);

        return status;
    }

    inline GLStatus linkAndUseProgram(GLuint program) {
        glLinkProgram(program);

        auto status = checkProgramLink(program);
        if (!status.success) {
            LogError(fmt::format("Error linking GL program: {}", status.message));
            return status;
        }
        
        glUseProgram(program);

        return status;
    }
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