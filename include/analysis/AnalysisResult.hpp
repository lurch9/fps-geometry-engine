#pragma once
#include <string>
#include <vector>

#include "analysis/ReachabilityAnalyzer.hpp"
#include "analysis/ExposureAnalyzer.hpp"
#include "analysis/VisibilityAnalyzer.hpp"

struct AnalysisResult {
  ReachabilityResult reachability;
  ExposureResult exposure;
  VisibilityResult visibility;

  std::vector<std::string> explanations; // at least 2: mechanical + factual
};
