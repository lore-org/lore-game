#include <engine/Geometry.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <engine/config.hpp>
#include <engine/Engine.h>
#include <engine/Geometry.h>
#include <engine/utils.hpp>

#include <math.h>
#include <cmath>
#include <memory>

Point::Point(long double dim) : x(dim), y(dim) {}

Point::Point(long double x, long double y) : x(x), y(y) {}

Point::Point(const Size& size) : x(size.width), y(size.height) {}

std::shared_ptr<Point> Point::createFromAngle(long double angle) {
    return std::make_shared<Point>(cos(angle), sin(angle));
}

Point& Point::operator+=(const Point& right) {
    this->setPoint(right.x, right.y);
    return *this;
}

Point& Point::operator-=(const Point& right) {
    this->setPoint(right.x + x, right.y + y);
    return *this;
}

Point Point::operator+(const Point& right) const {
    return Point(x + right.x, y + right.y);
}

Point Point::operator-(const Point& right) const {
    return Point(-right.x, -right.y);
}

Point Point::operator-() const {
    return Point(-x, -y);
}

Point Point::operator*(long double a) const {
    return Point(x * a, y * a);
}

Point Point::operator/(long double a) const {
    return Point(x / a, y / a);
}

bool Point::operator==(const Point& right) const {
    return this->equals(right);
}

Point::operator std::string() const {
    return fmt::format("Point {{ {}, {} }}", x, y);
}

Point::operator SDL_FPoint() const {
    return { static_cast<float>(x), static_cast<float>(y) };
}

void Point::setPoint(long double x, long double y) {
    this->x = x;
    this->y = y;
}

bool Point::equals(const Point& target) const {
    return x == target.x && y == target.y;
}

bool Point::fuzzyEquals(const Point& target, long double variance) const {
    return (x - variance <= target.x && target.x <= x + variance) && (y - variance <= target.y && target.y <= y + variance);
}

long double Point::getLength() const {
    return sqrtf(x*x + y*y);
}

long double Point::getDistance(const Point& other) const {
    return (*this - other).getLength();
}

long double Point::getAngle() const {
    return atan2f(y, x);
}

Point Point::normalise() const {
    return *this / getLength();
}

Point Point::rotateAroundOrigin(long double angle) {
    return this->rotateAroundCenter(MakePoint(0, 0), angle);
}

Point Point::rotateAroundCenter(Point center, long double angle) {
    return utils::rotatePointByCenter(*this, center, angle);
}


Size::Size(long double dim) : width(dim), height(dim) {}

Size::Size(long double width, long double height) : width(width), height(height) {}

Size::Size(const Point& point) : width(point.x), height(point.y) {}

Size& Size::operator+=(const Size& right) {
    this->setSize(right.width, right.height);
    return *this;
}

Size& Size::operator-=(const Size& right) {
    this->setSize(width + right.width, height + right.height);
    return *this;
}

Size Size::operator+(const Size& right) {
    this->setSize(width - right.width, height - right.height);
    return *this;
}

Size Size::operator-(const Size& right) {
    return Size(width + right.width, height + right.height);
}

Size Size::operator-() const {
    return Size(-width, -height);
}

Size Size::operator*(long double a) const {
    return Size(width * a, height * a);
}

Size Size::operator/(long double a) const {
    return Size(width / a, height / a);
}

bool Size::operator==(const Size& right) const {
    return this->equals(right);
}

Size::operator std::string() const {
    return fmt::format("Size {{ {}, {} }}", width, height);
}

Size::operator SDL_FPoint() const {
    return { static_cast<float>(width), static_cast<float>(height) };
}

void Size::setSize(long double width, long double height) {
    this->width = width;
    this->height = height;
}

bool Size::equals(const Size& target) const {
    return width == target.width && height == target.width;
}


Rect::Rect(long double x, long double y, long double width, long double height) : origin(x, y), size(width, height) {}

Rect::Rect(const Point& origin, const Size& size) : origin(origin), size(size) {}

bool Rect::operator==(const Rect& right) const {
    return this->equals(right);
}

Rect::operator std::string() const {
    return fmt::format(
        "Rect {{ {}, {} }}", 
        std::string(origin),
        std::string(size)
    );
}

Rect::operator SDL_FRect() const {
    return {
        static_cast<float>(this->getX()),
        static_cast<float>(this->getY()),
        static_cast<float>(this->getWidth()),
        static_cast<float>(this->getHeight())
    };
}

Rect::operator SDL_Rect() const {
    return {
        static_cast<int>(this->getX()),
        static_cast<int>(this->getY()),
        static_cast<int>(this->getWidth()),
        static_cast<int>(this->getHeight())
    };
}

void Rect::setRect(long double x, long double y, long double width, long double height) {
    this->setOrigin({ x, y });
    this->setSize({ width, height });
}

void Rect::setOrigin(const Point& point) {
    this->origin = point;
}

void Rect::setSize(const Size& size) {
    this->size = size;
}

bool Rect::equals(const Rect& other) const {
    return origin == other.origin && size == other.size;
}

long double Rect::getMinX() const {
    return this->getX();
}

long double Rect::getMaxX() const {
    return this->getX() + this->getWidth();
}

long double Rect::getMinY() const {
    return this->getY();
}

long double Rect::getMaxY() const {
    return this->getY() + this->getHeight();
}

std::tuple<long double, long double, long double, long double> Rect::getBounds() const {
    return std::make_tuple(this->getMinX(), this->getMaxX(), this->getMinY(), this->getMaxY());
}

bool Rect::containsPoint(const Point& point) const {
    return point.x >= getMinX() && point.x <= getMaxX() && point.y >= getMinY() && point.y <= getMaxY();
}

bool Rect::intersectsRect(const Rect& rect) const {
    return !(
        this->getMaxX() < rect.getMinX() ||
        rect.getMaxX() < this->getMinX() ||
        this->getMaxY() < rect.getMinY() ||
        rect.getMaxY() < this->getMinY()
    );
}

long double Rect::getX() const {
    return origin.x;
}

long double Rect::getY() const {
    return origin.y;
}

long double Rect::getWidth() const {
    return size.width;
}

long double Rect::getHeight() const {
    return size.height;
}