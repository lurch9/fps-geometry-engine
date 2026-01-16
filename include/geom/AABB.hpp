#pragma once
#include "geom/Vec2.hpp"

struct AABB {
  Vec2 min;
  Vec2 max;

  bool contains(const Vec2& p) const {
    return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
  }

  AABB inflated(double r) const {
    return AABB{Vec2{min.x - r, min.y - r}, Vec2{max.x + r, max.y + r}};
  }
};
