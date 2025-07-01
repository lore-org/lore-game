#pragma once

#include <math.h>
#include <tuple>
#include <memory>
#include <string>

#include <SDL3/SDL_rect.h>

class Size;

class Point : public std::enable_shared_from_this<Point> {
public:
    float x;
    float y;

    Point();
    Point(float dim);
    Point(float x, float y);
    Point(const Point& point);
    Point(const Size& size);

    // In Radians
    static std::shared_ptr<Point> createFromAngle(float angle);
    
    Point& operator+=(const Point& right);
    Point& operator-=(const Point& right);
    Point operator+(const Point& right) const;
    Point operator-(const Point& right) const;
    Point operator-() const;
    Point operator*(float a) const;
    Point operator/(float a) const;
    bool operator==(const Point& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setPoint(float x, float y);
    bool equals(const Point& target) const;
    
    bool fuzzyEquals(const Point& target, float variance) const;

    float getLength() const;

    float getDistance(const Point& other) const;

    float getAngle() const;

    Point normalise() const;

    Point rotateByAngle(float angle);
};

class Size : public std::enable_shared_from_this<Size> {
public:
    float width;
    float height;

    Size();
    Size(float dim);
    Size(float width, float height);
    Size(const Size& size);
    Size(const Point& point);

    Size& operator+=(const Size& right);
    Size& operator-=(const Size& right);
    Size operator+(const Size& right);
    Size operator-(const Size& right);
    Size operator-() const;
    Size operator*(float a) const;
    Size operator/(float a) const;
    bool operator==(const Size& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setSize(float width, float height);
    bool equals(const Size& target) const;
};

class Rect : public std::enable_shared_from_this<Rect> {
public:
    Point origin;
    Size size;

    Rect();
    Rect(float x, float y, float width, float height);
    Rect(const Point& origin, const Size& size);
    Rect(const Rect& other);

    bool operator==(const Rect& right) const;

    operator SDL_FRect() const;

    operator std::string() const;

    void setRect(float x, float y, float width, float height);
    void setOrigin(const Point& point);
    void setSize(const Size& size);

    bool equals(const Rect& other) const;
    float getMinX() const;
    float getMaxX() const;
    float getMinY() const;
    float getMaxY() const;
    std::tuple<float, float, float, float> getBounds() const;

    bool containsPoint(const Point& point) const;
    bool intersectsRect(const Rect& rect) const;

    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;
};