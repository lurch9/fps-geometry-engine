#include "analysis/ReachabilityAnalyzer.hpp"
#include <cmath>

namespace {

// Helper: sample grid points inside a circle
std::vector<Vec2> sampleReachable(
    const Map& map,
    const Vec2& center,
    double radius,
    double cellSize,
    double agentRadius)
{
  std::vector<Vec2> points;

  const int steps = static_cast<int>(std::ceil(radius / cellSize));

  for (int dx = -steps; dx <= steps; ++dx) {
    for (int dy = -steps; dy <= steps; ++dy) {
      Vec2 p {
        center.x + dx * cellSize,
        center.y + dy * cellSize
      };

      // radial check
      if ((p - center).norm() > radius)
        continue;

      // collision check
      if (map.collidesCircleAt(p, agentRadius))
        continue;

      points.push_back(p);
    }
  }

  return points;
}

} // anonymous namespace

ReachabilityResult ReachabilityAnalyzer::analyze(const Scene& scene) const {
  ReachabilityResult result;

  const double maxDistSelf   = scene.self.speed   * scene.T;
  const double maxDistEnemy  = scene.enemy.speed  * scene.T;

  result.reachableSelf = sampleReachable(
    scene.map,
    scene.self.pos,
    maxDistSelf,
    scene.cellSize,
    scene.self.radius
  );

  result.reachableEnemy = sampleReachable(
    scene.map,
    scene.enemy.pos,
    maxDistEnemy,
    scene.cellSize,
    scene.enemy.radius
  );

  if (!result.reachableEnemy.empty()) {
    result.areaRatio =
      static_cast<double>(result.reachableSelf.size()) /
      static_cast<double>(result.reachableEnemy.size());
  }

  return result;
}
