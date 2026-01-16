#pragma once
#include "core/Scene.hpp"

struct VisibilityResult {
  double visibleFraction = 0.0; // [0,1]
  int visibleCount = 0;
  int sampleCount = 0;
};

class VisibilityAnalyzer {
public:
  // shooter -> target circle visibility
  VisibilityResult analyze(const Scene& scene,
                           const Vec2& shooterPos,
                           const Agent& target) const;
};
