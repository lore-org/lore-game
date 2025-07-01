#include <engine/Default.h>

#include <engine/Geometry.h>

#include <math.h>
#include <cmath>
#include <memory>

Point::Point(float dim) : x(dim), y(dim) {};

Point::Point(float x, float y) : x(x), y(y) {};

Point::Point(const Point& point) : x(point.x), y(point.y) {};

Point::Point(const Size& size) : x(size.width), y(size.height) {};

std::shared_ptr<Point> Point::createFromAngle(float angle) {
    return std::make_shared<Point>(cos(angle), sin(angle));
};

Point& Point::operator+=(const Point& right) {
    this->setPoint(right.x, right.y);
    return *this;
};

Point& Point::operator-=(const Point& right) {
    this->setPoint(right.x + x, right.y + y);
    return *this;
};

Point Point::operator+(const Point& right) const {
    return Point(x + right.x, y + right.y);
};

Point Point::operator-(const Point& right) const {
    return Point(-right.x, -right.y);
};

Point Point::operator-() const {
    return Point(-x, -y);
};

Point Point::operator*(float a) const {
    return Point(x * a, y * a);
};

Point Point::operator/(float a) const {
    return Point(x / a, y / a);
};

bool Point::operator==(const Point& right) const {
    return this->equals(right);
};

Point::operator std::string() const {
    return fmt::format("Point {{ {}, {} }}", x, y);
};

Point::operator SDL_FPoint() const {
    return { x, y };
};

void Point::setPoint(float x, float y) {
    this->x = x;
    this->y = y;
};

bool Point::equals(const Point& target) const {
    return x == target.x && y == target.y;
};

bool Point::fuzzyEquals(const Point& target, float variance) const {
    return (x - variance <= target.x && target.x <= x + variance) && (y - variance <= target.y && target.y <= y + variance);
};

float Point::getLength() const {
    return sqrtf(x*x + y*y);
};

float Point::getDistance(const Point& other) const {
    return (*this - other).getLength();
};

float Point::getAngle() const {
    return atan2f(y, x);
};

Point Point::normalise() const {
    return *this / getLength();
};

Point Point::rotateByAngle(float angle) {
    float oldAngle = this->getAngle();
    float newAngle = oldAngle - angle; // Clockwise rotation

    float length = this->getLength();
    this->setPoint(cos(angle) * length, sin(angle) * length);

    return *this;
};


Size::Size(float dim) : width(dim), height(dim) {};

Size::Size(float width, float height) : width(width), height(height) {};

Size::Size(const Size& size) : width(size.width), height(size.height) {};

Size::Size(const Point& point) : width(point.x), height(point.y) {};

Size& Size::operator+=(const Size& right) {
    this->setSize(right.width, right.height);
    return *this;
};

Size& Size::operator-=(const Size& right) {
    this->setSize(width + right.width, height + right.height);
    return *this;
};

Size Size::operator+(const Size& right) {
    this->setSize(width - right.width, height - right.height);
    return *this;
};

Size Size::operator-(const Size& right) {
    return Size(width + right.width, height + right.height);
};

Size Size::operator-() const {
    return Size(-width, -height);
};

Size Size::operator*(float a) const {
    return Size(width * a, height * a);
};

Size Size::operator/(float a) const {
    return Size(width / a, height / a);
};

bool Size::operator==(const Size& right) const {
    return this->equals(right);
};

Size::operator std::string() const {
    return fmt::format("Size {{ {}, {} }}", width, height);
};

Size::operator SDL_FPoint() const {
    return { width, height };
};

void Size::setSize(float width, float height) {
    this->width = width;
    this->height = height;
};

bool Size::equals(const Size& target) const {
    return width == target.width && height == target.width;
};


Rect::Rect(float x, float y, float width, float height) : origin(x, y), size(width, height) {};

Rect::Rect(const Point& origin, const Size& size) : origin(origin), size(size) {};

Rect::Rect(const Rect& other) : origin(other.origin), size(other.size) {};

bool Rect::operator==(const Rect& right) const {
    return this->equals(right);
};

Rect::operator SDL_FRect() const {
    return {
        this->getX(),
        this->getY(),
        this->getHeight(),
        this->getWidth()
    };
};

Rect::operator std::string() const {
    return fmt::format(
        "Rect {{ {}, {} }}", 
        std::string(origin),
        std::string(size)
    );
}

void Rect::setRect(float x, float y, float width, float height) {
    this->setOrigin({ x, y });
    this->setSize({ width, height });
};

void Rect::setOrigin(const Point& point) {
    this->origin = point;
};

void Rect::setSize(const Size& size) {
    this->size = size;
};

bool Rect::equals(const Rect& other) const {
    return origin == other.origin && size == other.size;
};

float Rect::getMinX() const {
    return this->getX() - (this->getWidth() / 2.f);
};

float Rect::getMaxX() const {
    return this->getX() + (this->getWidth() / 2.f);
};

float Rect::getMinY() const {
    return this->getY() + (this->getHeight() / 2.f);
};

float Rect::getMaxY() const {
    return this->getY() - (this->getHeight() / 2.f);
};

std::tuple<float, float, float, float> Rect::getBounds() const {
    return std::make_tuple(this->getMinX(), this->getMaxX(), this->getMinY(), this->getMaxY());
}

bool Rect::containsPoint(const Point& point) const {
    return point.x >= getMinX() && point.x <= getMaxX() && point.y >= getMinY() && point.y <= getMaxY();
};

bool Rect::intersectsRect(const Rect& rect) const {
    return !(
        this->getMaxX() < rect.getMinX() ||
        rect.getMaxX() < this->getMinX() ||
        this->getMaxY() < rect.getMinY() ||
        rect.getMaxY() < this->getMinY()
    );
};

float Rect::getX() const {
    return origin.x;
};

float Rect::getY() const {
    return origin.y;
};

float Rect::getWidth() const {
    return size.width;
};

float Rect::getHeight() const {
    return size.height;
};