#pragma once

#include "analysis/AnalysisResult.hpp"
#include "core/Scene.hpp"

class SceneAnalyzer {
public:
  AnalysisResult analyze(const Scene& scene) const;
};
