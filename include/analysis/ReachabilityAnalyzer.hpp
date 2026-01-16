#pragma once

#include <vector>
#include "core/Scene.hpp"
#include "geom/Vec2.hpp"

// Result type kept simple and explicit for MVP
struct ReachabilityResult {
  std::vector<Vec2> reachableSelf;
  std::vector<Vec2> reachableEnemy;
  double areaRatio = 0.0;
};

class ReachabilityAnalyzer {
public:
  ReachabilityResult analyze(const Scene& scene) const;
};
