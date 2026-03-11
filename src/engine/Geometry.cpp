#include <furredengine/Geometry.h>

#include <math.h>
#include <cmath>
#include <memory>

#include <fmt/format.h>

#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>

using namespace FurredEngine;

FurredEngine::Point::Point(long double dim) : x(dim), y(dim) {}

FurredEngine::Point::Point(long double x, long double y) : x(x), y(y) {}

FurredEngine::Point::Point(const FurredEngine::Size& size) : x(size.width), y(size.height) {}

FurredEngine::Point FurredEngine::Point::createFromAngle(long double angle) {
    return MakePoint(std::cos(angle), std::sin(angle));
}

FurredEngine::Point& FurredEngine::Point::operator+=(const FurredEngine::Point& right) {
    this->setPoint(right.x, right.y);
    return *this;
}

FurredEngine::Point& FurredEngine::Point::operator-=(const FurredEngine::Point& right) {
    this->setPoint(right.x + x, right.y + y);
    return *this;
}

FurredEngine::Point FurredEngine::Point::operator+(const FurredEngine::Point& right) const {
    return FurredEngine::Point(x + right.x, y + right.y);
}

FurredEngine::Point FurredEngine::Point::operator-(const FurredEngine::Point& right) const {
    return FurredEngine::Point(-right.x, -right.y);
}

FurredEngine::Point FurredEngine::Point::operator-() const {
    return FurredEngine::Point(-x, -y);
}

FurredEngine::Point FurredEngine::Point::operator*(long double a) const {
    return FurredEngine::Point(x * a, y * a);
}

FurredEngine::Point FurredEngine::Point::operator/(long double a) const {
    return FurredEngine::Point(x / a, y / a);
}

bool FurredEngine::Point::operator==(const FurredEngine::Point& right) const {
    return this->equals(right);
}

FurredEngine::Point::operator std::string() const {
    return fmt::format("Point {{ {}, {} }}", x, y);
}

FurredEngine::Point::operator glm::vec2() const {
    return {
        static_cast<float>(x),
        static_cast<float>(y)
    };
}

void FurredEngine::Point::setPoint(long double x, long double y) {
    this->x = x;
    this->y = y;
}

bool FurredEngine::Point::equals(const FurredEngine::Point& target) const {
    return x == target.x && y == target.y;
}

bool FurredEngine::Point::fuzzyEquals(const FurredEngine::Point& target, long double variance) const {
    return (x - variance <= target.x && target.x <= x + variance) && (y - variance <= target.y && target.y <= y + variance);
}

long double FurredEngine::Point::getLength() const {
    return sqrtf(x*x + y*y);
}

long double FurredEngine::Point::getDistance(const FurredEngine::Point& other) const {
    return (*this - other).getLength();
}

long double FurredEngine::Point::getAngle() const {
    return atan2f(y, x);
}

FurredEngine::Point FurredEngine::Point::normalise() const {
    return *this / getLength();
}

FurredEngine::Point FurredEngine::Point::rotateAroundOrigin(long double angle) {
    return this->rotateAroundCenter(MakePoint(0, 0), angle);
}

FurredEngine::Point FurredEngine::Point::rotateAroundCenter(FurredEngine::Point center, long double angle) {
    return utils::rotatePointByCenter(*this, center, angle);
}


FurredEngine::Size::Size(long double dim) : width(dim), height(dim) {}

FurredEngine::Size::Size(long double width, long double height) : width(width), height(height) {}

FurredEngine::Size::Size(const FurredEngine::Point& point) : width(point.x), height(point.y) {}

FurredEngine::Size& FurredEngine::Size::operator+=(const FurredEngine::Size& right) {
    this->setSize(right.width, right.height);
    return *this;
}

FurredEngine::Size& FurredEngine::Size::operator-=(const FurredEngine::Size& right) {
    this->setSize(width + right.width, height + right.height);
    return *this;
}

FurredEngine::Size FurredEngine::Size::operator+(const FurredEngine::Size& right) {
    this->setSize(width - right.width, height - right.height);
    return *this;
}

FurredEngine::Size FurredEngine::Size::operator-(const FurredEngine::Size& right) {
    return FurredEngine::Size(width + right.width, height + right.height);
}

FurredEngine::Size FurredEngine::Size::operator-() const {
    return FurredEngine::Size(-width, -height);
}

FurredEngine::Size FurredEngine::Size::operator*(long double a) const {
    return FurredEngine::Size(width * a, height * a);
}

FurredEngine::Size FurredEngine::Size::operator/(long double a) const {
    return FurredEngine::Size(width / a, height / a);
}

bool FurredEngine::Size::operator==(const FurredEngine::Size& right) const {
    return this->equals(right);
}

FurredEngine::Size::operator std::string() const {
    return fmt::format("Size {{ {}, {} }}", width, height);
}

FurredEngine::Size::operator glm::vec2() const {
    return {
        static_cast<float>(width),
        static_cast<float>(height)
    };
}

void FurredEngine::Size::setSize(long double width, long double height) {
    this->width = width;
    this->height = height;
}

bool FurredEngine::Size::equals(const FurredEngine::Size& target) const {
    return width == target.width && height == target.width;
}


FurredEngine::Rect::Rect(long double x, long double y, long double width, long double height) : origin(x, y), size(width, height) {}

FurredEngine::Rect::Rect(const FurredEngine::Point& origin, const FurredEngine::Size& size) : origin(origin), size(size) {}

bool FurredEngine::Rect::operator==(const FurredEngine::Rect& right) const {
    return this->equals(right);
}

FurredEngine::Rect::operator std::string() const {
    return fmt::format(
        "Rect {{ {}, {} }}", 
        std::string(origin),
        std::string(size)
    );
}

FurredEngine::Rect::operator glm::vec4() const {
    return {
        static_cast<float>(origin.x),
        static_cast<float>(origin.y),
        static_cast<float>(size.width),
        static_cast<float>(size.height)
    };
}

void FurredEngine::Rect::setRect(long double x, long double y, long double width, long double height) {
    this->setOrigin({ x, y });
    this->setSize({ width, height });
}

void FurredEngine::Rect::setOrigin(const FurredEngine::Point& point) {
    this->origin = point;
}

void FurredEngine::Rect::setSize(const FurredEngine::Size& size) {
    this->size = size;
}

bool FurredEngine::Rect::equals(const FurredEngine::Rect& other) const {
    return origin == other.origin && size == other.size;
}

long double FurredEngine::Rect::getMinX() const {
    return this->getX();
}

long double FurredEngine::Rect::getMaxX() const {
    return this->getX() + this->getWidth();
}

long double FurredEngine::Rect::getMinY() const {
    return this->getY();
}

long double FurredEngine::Rect::getMaxY() const {
    return this->getY() + this->getHeight();
}

std::tuple<long double, long double, long double, long double> FurredEngine::Rect::getBounds() const {
    return std::make_tuple(this->getMinX(), this->getMaxX(), this->getMinY(), this->getMaxY());
}

bool FurredEngine::Rect::containsPoint(const FurredEngine::Point& point) const {
    return point.x >= getMinX() && point.x <= getMaxX() && point.y >= getMinY() && point.y <= getMaxY();
}

bool FurredEngine::Rect::intersectsRect(const FurredEngine::Rect& rect) const {
    return !(
        this->getMaxX() < rect.getMinX() ||
        rect.getMaxX() < this->getMinX() ||
        this->getMaxY() < rect.getMinY() ||
        rect.getMaxY() < this->getMinY()
    );
}

long double FurredEngine::Rect::getX() const {
    return origin.x;
}

long double FurredEngine::Rect::getY() const {
    return origin.y;
}

long double FurredEngine::Rect::getWidth() const {
    return size.width;
}

long double FurredEngine::Rect::getHeight() const {
    return size.height;
}