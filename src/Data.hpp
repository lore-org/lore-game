#pragma once

#include <algorithm>
#include <math.h>
#include <cmath>
#include <tuple>

#include <raylib-cpp.hpp>
#include <fmt/format.h>

class Size;

class Point {
public:
    float x;
    float y;

    Point();
    Point(float dim) : x(dim), y(dim) {};
    Point(float x, float y) : x(x), y(y) {};
    Point(const Point& point) : x(point.x), y(point.y) {};
    Point(const Size& size);

    // In Radians
    inline static Point* createFromAngle(float angle) {
        return new Point(cos(angle), sin(angle));
    };
    
    Point& operator=(const Point& right) {
        this->setPoint(right.x, right.y);
        return *this;
    };
    Point& operator+=(const Point& right) {
        this->setPoint(right.x + x, right.y + y);
        return *this;
    };
    Point& operator-=(const Point& right) {
        this->setPoint(right.x - x, right.y - y);
        return *this;
    };
    inline Point operator+(const Point& right) const {
        return Point(x + right.x, y + right.y);
    };
    inline Point operator-(const Point& right) const {
        return Point(x + right.x, y + right.y);
    };
    inline Point operator-() const {
        return Point(-x, -y);
    };
    inline Point operator*(float a) const {
        return Point(x * a, y * a);
    };
    inline Point operator/(float a) const {
        return Point(x / a, y / a);
    };
    inline bool operator==(const Point& right) const {
        return this->equals(right);
    };

    inline operator Vector2() const {
        return { x, y };
    }

    inline operator std::string() const {
        return fmt::format("Point {{ {}, {} }}", x, y);
    }

    void setPoint(float x, float y) {
        this->x = x;
        this->y = y;
    };
    inline bool equals(const Point& target) const {
        return x == target.x && y == target.y;
    };
    
    inline bool fuzzyEquals(const Point& target, float variance) const {
        return (x - variance <= target.x && target.x <= x + variance) && (y - variance <= target.y && target.y <= y + variance);
    };

    inline float getLength() const {
        return sqrtf(x*x + y*y);
    };

    inline float getDistance(const Point& other) const {
        return (*this - other).getLength();
    };

    inline float getAngle() const {
        return atan2f(y, x);
    };

    inline Point normalise() const {
        return *this / getLength();
    };

    Point rotateByAngle(float angle) {
        float oldAngle = this->getAngle();
        float newAngle = oldAngle - angle; // clockwise rotation

        float length = this->getLength();
        this->setPoint(cos(angle) * length, sin(angle) * length);

        return *this;
    };
};

class Size {
public:
    float width;
    float height;

    Size();
    Size(float dim) : width(dim), height(dim) {};
    Size(float width, float height) : width(width), height(height) {};
    Size(const Size& size) : width(size.width), height(size.height) {};
    Size(const Point& point) : width(point.x), height(point.y) {};

    Size& operator=(const Size& right) {
        this->setSize(right.width, right.height);
        return *this;
    };
    Size& operator+=(const Size& right) {
        this->setSize(width + right.width, height + right.height);
        return *this;
    };
    Size& operator-=(const Size& right) {
        this->setSize(width - right.width, height - right.height);
        return *this;
    };
    inline Size operator+(const Size& right) const {
        return Size(width + right.width, height + right.height);
    };
    inline Size operator-(const Size& right) const {
        return Size(width - right.width, height - right.height);
    };
    inline Size operator-() const {
        return Size(-width, -height);
    };
    inline Size operator*(float a) const {
        return Size(width * a, height * a);
    };
    inline Size operator/(float a) const {
        return Size(width / a, height / a);
    };
    inline bool operator==(const Size& right) const {
        return this->equals(right);
    };

    inline operator Vector2() const {
        return { width, height };
    }

    inline operator std::string() const {
        return fmt::format("Size {{ {}, {} }}", width, height);
    }

    void setSize(float width, float height) {
        this->width = width;
        this->height = height;
    };
    inline bool equals(const Size& target) const {
        return width == target.width && height == target.width;
    };
};

inline Point::Point(const Size& size) : x(size.width), y(size.height) {}

class Rect {
public:
    Point origin;
    Size size;

    Rect();
    Rect(float x, float y, float width, float height) : origin(x, y), size(width, height) {};
    Rect(const Point& origin, const Size& size) : origin(origin), size(size) {};
    Rect(const Rect& other) : origin(other.origin), size(other.size) {};

    Rect& operator=(const Rect& right) {
        this->setOrigin(right.origin);
        this->setSize(right.size);
        return *this;
    };
    inline bool operator==(const Rect& right) const {
        return this->equals(right);
    };

    inline operator Vector4() const {
        return {
            this->getX(),
            this->getY(),
            this->getWidth(),
            this->getHeight()
        };
    }

    inline operator Rectangle() const {
        return {
            this->getX(),
            this->getY(),
            this->getWidth(),
            this->getHeight()
        };
    }

    inline operator std::string() const {
        return fmt::format(
            "Rect {{ {}, {} }}", 
            this->getX(),
            this->getY(),
            this->getWidth(),
            this->getHeight()
        );
    }

    void setRect(float x, float y, float width, float height) {
        this->setOrigin({ x, y });
        this->setSize({ width, height });
    };
    void setOrigin(const Point& point) {
        this->origin = point;
    };
    void setSize(const Size& size) {
        this->size = size;
    };

    inline bool equals(const Rect& other) const {
        return origin == other.origin && size == other.size;
    };
    inline float getMinX() const {
        return this->getX() - (this->getWidth() / 2.f);
    };
    inline float getMaxX() const {
        return this->getX() + (this->getWidth() / 2.f);
    };
    inline float getMinY() const {
        return this->getY() + (this->getHeight() / 2.f);
    };
    inline float getMaxY() const {
        return this->getY() - (this->getHeight() / 2.f);
    };
    inline std::tuple<float, float, float, float> getBounds() const {
        return std::make_tuple(this->getMinX(), this->getMaxX(), this->getMinY(), this->getMaxY());
    }

    inline bool containsPoint(const Point& point) const {
        return point.x >= getMinX() && point.x <= getMaxX() && point.y >= getMinY() && point.y <= getMaxY();
    };
    inline bool intersectsRect(const Rect& rect) const {
        return !(
            this->getMaxX() < rect.getMinX() ||
            rect.getMaxX() < this->getMinX() ||
            this->getMaxY() < rect.getMinY() ||
            rect.getMaxY() < this->getMinY()
        );
    };

    inline float getX() const {
        return origin.x;
    };
    inline float getY() const {
        return origin.y;
    };
    inline float getWidth() const {
        return size.width;
    };
    inline float getHeight() const {
        return size.height;
    };
};

#define MakePoint(x, y) Point((float)x, (float)y)
#define MakeSize(width, height) Size((float)width, (float)height)
#define MakeRect(x, y, width, height) Rect((float)x, (float)y, (float)width, (float)height)

#ifndef EPSILON
#define EPSILON 0.000001f
#endif
#define IsZero(f) std::fabs((float)f) < EPSILON

namespace utils {
    inline std::string toLowerCase(std::string str) {
        std::string ret;
        std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
        return str;
    }
}