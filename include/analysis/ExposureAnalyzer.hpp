#pragma once
#include "core/Scene.hpp"

struct ExposureResult {
  double width = 0.0;
  int losCount = 0;
  int totalEnemyReachable = 0;
};

class ExposureAnalyzer {
public:
  ExposureResult analyze(const Scene& scene,
                         const std::vector<Vec2>& enemyReachable) const;
};
