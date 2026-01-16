#pragma once
#include "geom/AABB.hpp"
#include <algorithm>

// Segment (p0->p1) intersects AABB (including boundaries).
inline bool segmentIntersectsAABB(const Vec2& p0, const Vec2& p1, const AABB& b) {
  // Slab method for parametric segment: p(t) = p0 + t*(p1-p0), t in [0,1]
  const Vec2 d = p1 - p0;

  double tmin = 0.0;
  double tmax = 1.0;

  auto update = [&](double p, double q) -> bool {
    // p is direction component, q is difference to bound
    if (std::abs(p) < 1e-12) {
      return q >= 0.0; // parallel; must be inside slab
    }
    const double t = q / p;
    if (p < 0.0) {
      tmin = std::max(tmin, t);
    } else {
      tmax = std::min(tmax, t);
    }
    return tmin <= tmax;
  };

  // x slabs: p0.x + t*d.x between [min.x, max.x]
  if (!update(-d.x, p0.x - b.min.x)) return false;
  if (!update( d.x, b.max.x - p0.x)) return false;

  // y slabs
  if (!update(-d.y, p0.y - b.min.y)) return false;
  if (!update( d.y, b.max.y - p0.y)) return false;

  return true;
}
