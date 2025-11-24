#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Geometry.h>

#include <numbers>
#include <string>
#include <algorithm>
#include <vector>
#include <regex>
#include <cctype>

#include <SDL3/SDL_log.h>

#include "Object.h"

#define MakePoint(x, y) Point(static_cast<long double>(x), static_cast<long double>(y))
#define MakeSize(width, height) Size(static_cast<long double>(width), static_cast<long double>(height))
#define MakeRect(x, y, width, height) Rect(static_cast<long double>(x), static_cast<long double>(y), static_cast<long double>(width), static_cast<long double>(height))

#define EPSILON std::numeric_limits<long double>::epsilon()
#define IsZero(f) std::fabs(static_cast<long double>(f)) < EPSILON

#define GetLine() fmt::format("{}:{}", __FILE__, __LINE__).c_str()
#define GetWithTag(tag, x) fmt::format("[{}]: '{}': {}", tag, GetLine(), x).c_str()

#define Log(x) SDL_Log(x, 0)
#define LogCritical(x) SDL_LogCritical( SDL_LOG_CATEGORY_APPLICATION, GetWithTag("CRITICAL", x), 0 )
#define LogDebug(x)    SDL_LogDebug(    SDL_LOG_CATEGORY_APPLICATION, GetWithTag("DEBUG",    x), 0 )
#define LogError(x)    SDL_LogError(    SDL_LOG_CATEGORY_APPLICATION, GetWithTag("ERROR",    x), 0 )
#define LogInfo(x)     SDL_LogInfo(     SDL_LOG_CATEGORY_APPLICATION, GetWithTag("INFO",     x), 0 )
#define LogTrace(x)    SDL_LogTrace(    SDL_LOG_CATEGORY_APPLICATION, GetWithTag("TRACE",    x), 0 )
#define LogVerbose(x)  SDL_LogVerbose(  SDL_LOG_CATEGORY_APPLICATION, GetWithTag("VERBOSE",  x), 0 )
#define LogWarn(x)     SDL_LogWarn(     SDL_LOG_CATEGORY_APPLICATION, GetWithTag("WARN",     x), 0 )

#define LogSDLError() LogError(SDL_GetError())

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
            hash <<= sizeof(uintmax_t) * 4;
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

        auto cos = std::cosl(radians);
        auto sin = std::sinl(radians);

        auto translatedX = point.x - center.x;
        auto translatedY = point.y - center.y;

        return {
            (translatedX * cos - translatedY * sin) + center.x,
            (translatedX * sin + translatedY * cos) + center.y
        };
    }
}