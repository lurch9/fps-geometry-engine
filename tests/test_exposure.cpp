#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "analysis/ReachabilityAnalyzer.hpp"
#include "analysis/ExposureAnalyzer.hpp"
#include "core/Scene.hpp"
#include "geom/AABB.hpp"

TEST_CASE("Exposure width ~ diameter in empty map", "[exposure]") {
  Scene scene;
  scene.map.setWorldBounds(AABB{Vec2{0,0}, Vec2{10,10}});
  scene.T = 0.30;
  scene.cellSize = 0.5;

  scene.self.pos = Vec2{2,2};
  scene.self.facing = Vec2{1,0};

  scene.enemy.pos = Vec2{8,8};
  scene.enemy.speed = 5.0;
  scene.enemy.radius = 0.25;

  ReachabilityAnalyzer r;
  auto reach = r.analyze(scene);

  ExposureAnalyzer e;
  auto ex = e.analyze(scene, reach.reachableEnemy);

  // In empty space, most points should be LoS.
  REQUIRE(ex.losCount > 0);

  // Width should be close to 2 * v*T = 3.0 (grid tolerance).
  REQUIRE_THAT(ex.width, Catch::Matchers::WithinAbs(3.0, 0.75));
}
