#pragma once
#include <vector>
#include "geom/AABB.hpp"
#include "geom/Vec2.hpp"

class Map {
public:
  Map() = default;

  void setWorldBounds(const AABB& bounds) { worldBounds_ = bounds; }
  const AABB& worldBounds() const { return worldBounds_; }

  void addObstacle(const AABB& aabb) { obstacles_.push_back(aabb); }
  const std::vector<AABB>& obstacles() const { return obstacles_; }

  bool inBounds(const Vec2& p) const { return worldBounds_.contains(p); }

  bool hasLineOfSight(const Vec2& from, const Vec2& to) const;

  bool collidesCircleAt(const Vec2& center, double radius) const;

    // Editor helpers (viewer needs to mutate obstacles)
  std::vector<AABB>& obstaclesMutable() { return obstacles_; }
  void removeObstacle(size_t i) { obstacles_.erase(obstacles_.begin() + i); }
  void setObstacle(size_t i, const AABB& b) { obstacles_[i] = b; }


private:
  AABB worldBounds_{Vec2{0,0}, Vec2{10,10}};
  std::vector<AABB> obstacles_;
};
