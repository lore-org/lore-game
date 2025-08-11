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

Point::Point(double dim) : x(dim), y(dim) {}

Point::Point(double x, double y) : x(x), y(y) {}

Point::Point(const Size& size) : x(size.width), y(size.height) {}

std::shared_ptr<Point> Point::createFromAngle(double angle) {
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

Point Point::operator*(double a) const {
    return Point(x * a, y * a);
}

Point Point::operator/(double a) const {
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

void Point::setPoint(double x, double y) {
    this->x = x;
    this->y = y;
}

bool Point::equals(const Point& target) const {
    return x == target.x && y == target.y;
}

bool Point::fuzzyEquals(const Point& target, double variance) const {
    return (x - variance <= target.x && target.x <= x + variance) && (y - variance <= target.y && target.y <= y + variance);
}

double Point::getLength() const {
    return sqrtf(x*x + y*y);
}

double Point::getDistance(const Point& other) const {
    return (*this - other).getLength();
}

double Point::getAngle() const {
    return atan2f(y, x);
}

Point Point::normalise() const {
    return *this / getLength();
}

Point Point::rotateByAngle(double angle) {
    double oldAngle = this->getAngle();
    double newAngle = oldAngle - angle; // Clockwise rotation

    double length = this->getLength();
    this->setPoint(cos(angle) * length, sin(angle) * length);

    return *this;
}


Size::Size(double dim) : width(dim), height(dim) {}

Size::Size(double width, double height) : width(width), height(height) {}

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

Size Size::operator*(double a) const {
    return Size(width * a, height * a);
}

Size Size::operator/(double a) const {
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

void Size::setSize(double width, double height) {
    this->width = width;
    this->height = height;
}

bool Size::equals(const Size& target) const {
    return width == target.width && height == target.width;
}


Rect::Rect(double x, double y, double width, double height) : origin(x, y), size(width, height) {}

Rect::Rect(const Point& origin, const Size& size) : origin(origin), size(size) {}

bool Rect::operator==(const Rect& right) const {
    return this->equals(right);
}

Rect::operator SDL_FRect() const {
    return {
        static_cast<float>(this->getX()),
        static_cast<float>(this->getY()),
        static_cast<float>(this->getHeight()),
        static_cast<float>(this->getWidth())
    };
}

Rect::operator std::string() const {
    return fmt::format(
        "Rect {{ {}, {} }}", 
        std::string(origin),
        std::string(size)
    );
}

void Rect::setRect(double x, double y, double width, double height) {
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

double Rect::getMinX() const {
    return this->getX() - (this->getWidth() / 2.f);
}

double Rect::getMaxX() const {
    return this->getX() + (this->getWidth() / 2.f);
}

double Rect::getMinY() const {
    return this->getY() + (this->getHeight() / 2.f);
}

double Rect::getMaxY() const {
    return this->getY() - (this->getHeight() / 2.f);
}

std::tuple<double, double, double, double> Rect::getBounds() const {
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

double Rect::getX() const {
    return origin.x;
}

double Rect::getY() const {
    return origin.y;
}

double Rect::getWidth() const {
    return size.width;
}

double Rect::getHeight() const {
    return size.height;
}