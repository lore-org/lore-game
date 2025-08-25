#pragma once

#include <math.h>
#include <tuple>
#include <memory>
#include <string>

#include <SDL3/SDL_rect.h>

class Size;

class Point : public std::enable_shared_from_this<Point> {
public:
    long double x;
    long double y;

    Point();
    Point(long double dim);
    Point(long double x, long double y);
    Point(const Size& size);

    // In Radians
    static std::shared_ptr<Point> createFromAngle(long double angle);
    
    Point& operator+=(const Point& right);
    Point& operator-=(const Point& right);
    Point operator+(const Point& right) const;
    Point operator-(const Point& right) const;
    Point operator-() const;
    Point operator*(long double a) const;
    Point operator/(long double a) const;
    bool operator==(const Point& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setPoint(long double x, long double y);
    bool equals(const Point& target) const;
    
    bool fuzzyEquals(const Point& target, long double variance) const;

    long double getLength() const;

    long double getDistance(const Point& other) const;

    long double getAngle() const;

    Point normalise() const;

    Point rotateByAngle(long double angle);
};

class Size : public std::enable_shared_from_this<Size> {
public:
    long double width;
    long double height;

    Size();
    Size(long double dim);
    Size(long double width, long double height);
    Size(const Point& point);

    Size& operator+=(const Size& right);
    Size& operator-=(const Size& right);
    Size operator+(const Size& right);
    Size operator-(const Size& right);
    Size operator-() const;
    Size operator*(long double a) const;
    Size operator/(long double a) const;
    bool operator==(const Size& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setSize(long double width, long double height);
    bool equals(const Size& target) const;
};

class Rect : public std::enable_shared_from_this<Rect> {
public:
    Point origin;
    Size size;

    Rect();
    Rect(long double x, long double y, long double width, long double height);
    Rect(const Point& origin, const Size& size);

    bool operator==(const Rect& right) const;

    operator SDL_FRect() const;

    operator std::string() const;

    void setRect(long double x, long double y, long double width, long double height);
    void setOrigin(const Point& point);
    void setSize(const Size& size);

    bool equals(const Rect& other) const;
    long double getMinX() const;
    long double getMaxX() const;
    long double getMinY() const;
    long double getMaxY() const;
    std::tuple<long double, long double, long double, long double> getBounds() const;

    bool containsPoint(const Point& point) const;
    bool intersectsRect(const Rect& rect) const;

    long double getX() const;
    long double getY() const;
    long double getWidth() const;
    long double getHeight() const;
};