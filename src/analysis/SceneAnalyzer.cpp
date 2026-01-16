#include "analysis/SceneAnalyzer.hpp"

#include <sstream>
#include <iomanip>

#include "analysis/ReachabilityAnalyzer.hpp"
#include "analysis/ExposureAnalyzer.hpp"
#include "analysis/VisibilityAnalyzer.hpp"

AnalysisResult SceneAnalyzer::analyze(const Scene& scene) const {
  AnalysisResult out;

  ReachabilityAnalyzer reach;
  ExposureAnalyzer exposure;
  VisibilityAnalyzer visibility;

  // A) Reachable Area Ratio
  out.reachability = reach.analyze(scene);

  // B) Exposure Width (enemy reachable cells that have LoS from self)
  out.exposure = exposure.analyze(scene, out.reachability.reachableEnemy);

  // C) Visible Hit Fraction (self -> enemy)
  out.visibility = visibility.analyze(scene, scene.self.pos, scene.enemy);

  // --- Explainability strings ---
  {
    std::ostringstream mech;
    mech
      << "Mechanical: Reachability samples grid points (cellSize=" << scene.cellSize
      << ") inside movement disks (r=v*T, T=" << scene.T
      << "), rejects points colliding with obstacles inflated by agent radius. "
      << "Line-of-sight uses segment-vs-AABB intersection; out-of-bounds counts as blocked.";
    out.explanations.push_back(mech.str());
  }

  {
    std::ostringstream fact;
    fact << std::fixed << std::setprecision(3)
         << "Factual: Self reachable cells=" << out.reachability.reachableSelf.size()
         << ", Enemy reachable cells=" << out.reachability.reachableEnemy.size()
         << ", areaRatio=" << out.reachability.areaRatio
         << ". Enemy LoS-reachable-from-self=" << out.exposure.losCount
         << "/" << out.exposure.totalEnemyReachable
         << ", exposureWidth=" << out.exposure.width
         << ". VisibleHitFraction=" << out.visibility.visibleFraction
         << " (" << out.visibility.visibleCount << "/" << out.visibility.sampleCount << ").";
    out.explanations.push_back(fact.str());
  }

  return out;
}
