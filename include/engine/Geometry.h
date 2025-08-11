#pragma once

#include <math.h>
#include <tuple>
#include <memory>
#include <string>

#include <SDL3/SDL_rect.h>

class Size;

class Point : public std::enable_shared_from_this<Point> {
public:
    double x;
    double y;

    Point();
    Point(double dim);
    Point(double x, double y);
    Point(const Size& size);

    // In Radians
    static std::shared_ptr<Point> createFromAngle(double angle);
    
    Point& operator+=(const Point& right);
    Point& operator-=(const Point& right);
    Point operator+(const Point& right) const;
    Point operator-(const Point& right) const;
    Point operator-() const;
    Point operator*(double a) const;
    Point operator/(double a) const;
    bool operator==(const Point& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setPoint(double x, double y);
    bool equals(const Point& target) const;
    
    bool fuzzyEquals(const Point& target, double variance) const;

    double getLength() const;

    double getDistance(const Point& other) const;

    double getAngle() const;

    Point normalise() const;

    Point rotateByAngle(double angle);
};

class Size : public std::enable_shared_from_this<Size> {
public:
    double width;
    double height;

    Size();
    Size(double dim);
    Size(double width, double height);
    Size(const Point& point);

    Size& operator+=(const Size& right);
    Size& operator-=(const Size& right);
    Size operator+(const Size& right);
    Size operator-(const Size& right);
    Size operator-() const;
    Size operator*(double a) const;
    Size operator/(double a) const;
    bool operator==(const Size& right) const;

    operator std::string() const;
    operator SDL_FPoint() const;

    void setSize(double width, double height);
    bool equals(const Size& target) const;
};

class Rect : public std::enable_shared_from_this<Rect> {
public:
    Point origin;
    Size size;

    Rect();
    Rect(double x, double y, double width, double height);
    Rect(const Point& origin, const Size& size);

    bool operator==(const Rect& right) const;

    operator SDL_FRect() const;

    operator std::string() const;

    void setRect(double x, double y, double width, double height);
    void setOrigin(const Point& point);
    void setSize(const Size& size);

    bool equals(const Rect& other) const;
    double getMinX() const;
    double getMaxX() const;
    double getMinY() const;
    double getMaxY() const;
    std::tuple<double, double, double, double> getBounds() const;

    bool containsPoint(const Point& point) const;
    bool intersectsRect(const Rect& rect) const;

    double getX() const;
    double getY() const;
    double getWidth() const;
    double getHeight() const;
};