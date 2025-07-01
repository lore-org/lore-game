#pragma once
#include "Default.h" // IWYU pragma: keep

#include <string>
#include <algorithm>
#include <vector>

#include "Object.h"

#define MakePoint(x, y) Point(static_cast<float>(x), static_cast<float>(y))
#define MakeSize(width, height) Size(static_cast<float>(width), static_cast<float>(height))
#define MakeRect(x, y, width, height) Rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height))

#define EPSILON 0.000001f
#define IsZero(f) std::fabs(static_cast<float>(f)) < EPSILON

#define PrintLN() fmt::println("{}:{}", __FILE__, __LINE__)
#define PrintSDLError() fmt::println("[ERROR]: '{}:{}': {}", __FILE__, __LINE__, SDL_GetError())

#define __Quote__(...) #__VA_ARGS__
#define CreateEventDecl(prefix, event) inline static const char* event = __Quote__(prefix ## __ ## event)

namespace utils {
    inline std::string toLowerCase(std::string str) {
        std::string ret;
        std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
        return str;
    }

    template<typename T, typename U, typename F>
    inline std::vector<T> mapVector(std::vector<U> vec, F replace) {
        std::vector<T> newVec;
        std::transform(vec.begin(), vec.end(), newVec.begin(), replace);
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
}