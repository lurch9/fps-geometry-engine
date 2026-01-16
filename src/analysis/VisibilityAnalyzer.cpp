#include "analysis/VisibilityAnalyzer.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

VisibilityResult VisibilityAnalyzer::analyze(const Scene& scene,
                                            const Vec2& shooterPos,
                                            const Agent& target) const {
  VisibilityResult out;

  const int N = (scene.visibilitySamples > 0) ? scene.visibilitySamples : 1;
  out.sampleCount = N;

  // If shooter is out of bounds, we treat as no visibility (consistent with Map::hasLineOfSight)
  // If target center out of bounds, same outcome anyway because sampled points will be out.
  for (int i = 0; i < N; ++i) {
    const double theta = (2.0 * M_PI * static_cast<double>(i)) / static_cast<double>(N);

    const Vec2 sample{
      target.pos.x + std::cos(theta) * target.radius,
      target.pos.y + std::sin(theta) * target.radius
    };

    if (scene.map.hasLineOfSight(shooterPos, sample)) {
      out.visibleCount++;
    }
  }

  out.visibleFraction = static_cast<double>(out.visibleCount) / static_cast<double>(out.sampleCount);
  return out;
}
