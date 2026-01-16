#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "analysis/VisibilityAnalyzer.hpp"
#include "core/Scene.hpp"
#include "geom/AABB.hpp"

TEST_CASE("Visibility is ~1.0 in empty map", "[visibility]") {
  Scene scene;
  scene.map.setWorldBounds(AABB{Vec2{0,0}, Vec2{10,10}});
  scene.visibilitySamples = 64;

  scene.self.pos = Vec2{2,2};
  scene.enemy.pos = Vec2{8,8};
  scene.enemy.radius = 0.25;

  VisibilityAnalyzer v;
  auto res = v.analyze(scene, scene.self.pos, scene.enemy);

  REQUIRE(res.sampleCount == 64);
  REQUIRE_THAT(res.visibleFraction, Catch::Matchers::WithinAbs(1.0, 1e-9));
}

TEST_CASE("Obstacle reduces visibility fraction", "[visibility]") {
  Scene scene;
  scene.map.setWorldBounds(AABB{Vec2{0,0}, Vec2{10,10}});
  scene.visibilitySamples = 64;

  scene.self.pos = Vec2{2,5};
  scene.enemy.pos = Vec2{8,5};
  scene.enemy.radius = 0.5;

  // Big wall between them
  scene.map.addObstacle(AABB{Vec2{4.5, 0.0}, Vec2{5.5, 10.0}});

  VisibilityAnalyzer v;
  auto res = v.analyze(scene, scene.self.pos, scene.enemy);

  REQUIRE(res.visibleFraction < 0.25); // should be near 0 because wall blocks all rays
}
