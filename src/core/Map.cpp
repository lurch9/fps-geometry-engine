#include "core/Map.hpp"
#include "geom/Raycast.hpp"

bool Map::hasLineOfSight(const Vec2& from, const Vec2& to) const {
  // If either point is out of bounds, treat as no LoS for MVP.
  if (!inBounds(from) || !inBounds(to)) return false;

  for (const auto& ob : obstacles_) {
    if (segmentIntersectsAABB(from, to, ob)) {
      return false;
    }
  }
  return true;
}

bool Map::collidesCircleAt(const Vec2& center, double radius) const {
  if (!inBounds(center)) return true;

  for (const auto& ob : obstacles_) {
    // Inflate obstacle by radius: then circle-center inside inflated box => overlap.
    const AABB inflated = ob.inflated(radius);
    if (inflated.contains(center)) return true;
  }
  return false; 
}
