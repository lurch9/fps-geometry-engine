#pragma once
#include <cmath>

struct Vec2 {
  double x{0.0};
  double y{0.0};

  Vec2() = default;
  Vec2(double x_, double y_) : x(x_), y(y_) {}

  Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
  Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
  Vec2 operator*(double s) const { return {x * s, y * s}; }

  double dot(const Vec2& o) const { return x * o.x + y * o.y; }
  double norm() const { return std::sqrt(x * x + y * y); }

  Vec2 normalized() const {
    const double n = norm();
    return (n > 0.0) ? Vec2{x / n, y / n} : Vec2{1.0, 0.0};
  }
};

inline Vec2 perp(const Vec2& v) { return Vec2{-v.y, v.x}; }
inline double dist(const Vec2& a, const Vec2& b) { return (a - b).norm(); }
