#include "analysis/ExposureAnalyzer.hpp"
#include "geom/Vec2.hpp"
#include <limits>

ExposureResult ExposureAnalyzer::analyze(const Scene& scene,
                                         const std::vector<Vec2>& enemyReachable) const {
  ExposureResult out;
  out.totalEnemyReachable = static_cast<int>(enemyReachable.size());
  if (enemyReachable.empty()) return out;

  const Vec2 f = scene.self.facing.normalized();
  const Vec2 axis = perp(f);

  double minS = std::numeric_limits<double>::infinity();
  double maxS = -std::numeric_limits<double>::infinity();

  for (const auto& p : enemyReachable) {
    if (!scene.map.hasLineOfSight(scene.self.pos, p)) continue;

    out.losCount++;
    const double s = (p - scene.self.pos).dot(axis);
    minS = std::min(minS, s);
    maxS = std::max(maxS, s);
  }

  if (out.losCount > 0) out.width = (maxS - minS);
  return out;
}
