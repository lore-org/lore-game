#pragma once
#include "Default.hpp"

#include <string>
#include <algorithm>
#include <vector>

#define MakePoint(x, y) Point((float)x, (float)y)
#define MakeSize(width, height) Size((float)width, (float)height)
#define MakeRect(x, y, width, height) Rect((float)x, (float)y, (float)width, (float)height)

#define EPSILON 0.000001f
#define IsZero(f) std::fabs((float)f) < EPSILON

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
}