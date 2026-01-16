#include <iostream>
#include "analysis/SceneAnalyzer.hpp"
#include "geom/AABB.hpp"

int main() {
  Scene scene;
  scene.map.setWorldBounds(AABB{Vec2{0,0}, Vec2{10,10}});

  scene.T = 0.30;
  scene.cellSize = 0.5;
  scene.visibilitySamples = 64;

  scene.self.pos = Vec2{2,2};
  scene.self.facing = Vec2{1,0};
  scene.self.speed = 5.0;
  scene.self.radius = 0.25;

  scene.enemy.pos = Vec2{8,8};
  scene.enemy.facing = Vec2{-1,0};
  scene.enemy.speed = 5.0;
  scene.enemy.radius = 0.25;

  SceneAnalyzer analyzer;
  auto result = analyzer.analyze(scene);

  std::cout << "=== OPEN MAP SCENE ===\n";
  std::cout << "Reachable Area Ratio: " << result.reachability.areaRatio << "\n";
  std::cout << "Exposure Width: " << result.exposure.width << "\n";
  std::cout << "Visible Hit Fraction: " << result.visibility.visibleFraction << "\n\n";

  for (const auto& e : result.explanations)
    std::cout << "- " << e << "\n";

  return 0;
}
