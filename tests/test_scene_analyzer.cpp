#include <catch2/catch_test_macros.hpp>

#include "analysis/SceneAnalyzer.hpp"
#include "geom/AABB.hpp"

TEST_CASE("SceneAnalyzer returns all metrics and explanations", "[scene_analyzer]") {
  Scene scene;
  scene.map.setWorldBounds(AABB{Vec2{0,0}, Vec2{10,10}});
  scene.T = 0.30;
  scene.cellSize = 0.5;
  scene.visibilitySamples = 64;

  scene.self.pos = Vec2{2,2};
  scene.self.radius = 0.25;
  scene.self.speed = 5.0;
  scene.self.facing = Vec2{1,0};

  scene.enemy.pos = Vec2{8,8};
  scene.enemy.radius = 0.25;
  scene.enemy.speed = 5.0;
  scene.enemy.facing = Vec2{-1,0};

  SceneAnalyzer analyzer;
  auto res = analyzer.analyze(scene);

  REQUIRE_FALSE(res.reachability.reachableSelf.empty());
  REQUIRE_FALSE(res.reachability.reachableEnemy.empty());
  REQUIRE(res.explanations.size() >= 2);
}
