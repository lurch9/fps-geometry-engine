#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


#include "analysis/ReachabilityAnalyzer.hpp"
#include "core/Scene.hpp"
#include "geom/AABB.hpp"

TEST_CASE("Reachability returns non-empty regions in empty map", "[reachability]") {
  Scene scene;

  // World bounds
  scene.map.setWorldBounds(AABB{Vec2{0, 0}, Vec2{10, 10}});

  // Parameters
  scene.T = 0.30;
  scene.cellSize = 0.5;

  // Self agent
  scene.self.pos = Vec2{2, 2};
  scene.self.radius = 0.25;
  scene.self.speed = 5.0;
  scene.self.facing = Vec2{1, 0};

  // Enemy agent
  scene.enemy.pos = Vec2{8, 8};
  scene.enemy.radius = 0.25;
  scene.enemy.speed = 5.0;
  scene.enemy.facing = Vec2{-1, 0};

  ReachabilityAnalyzer analyzer;
  auto result = analyzer.analyze(scene);

  REQUIRE_FALSE(result.reachableSelf.empty());
  REQUIRE_FALSE(result.reachableEnemy.empty());

  // Same speeds => reachable sizes should be close.
  // We allow some tolerance because grid sampling can introduce minor differences.
  REQUIRE_THAT(result.areaRatio, Catch::Matchers::WithinAbs(1.0, 0.25));

}

TEST_CASE("Obstacle reduces reachable region", "[reachability]") {
  Scene scene;

  scene.map.setWorldBounds(AABB{Vec2{0, 0}, Vec2{10, 10}});

  scene.T = 0.30;
  scene.cellSize = 0.5;

  scene.self.pos = Vec2{2, 2};
  scene.self.radius = 0.25;
  scene.self.speed = 5.0;
  scene.self.facing = Vec2{1, 0};

  scene.enemy.pos = Vec2{8, 8};
  scene.enemy.radius = 0.25;
  scene.enemy.speed = 5.0;
  scene.enemy.facing = Vec2{-1, 0};

  // Analyze with no obstacles
  ReachabilityAnalyzer analyzer;
  auto baseline = analyzer.analyze(scene);

  // Add a big obstacle near Self to reduce valid cells
  scene.map.addObstacle(AABB{Vec2{1.0, 1.0}, Vec2{3.5, 3.5}});

  auto blocked = analyzer.analyze(scene);

  // Self should lose reachable points due to obstacle inflation by radius.
  REQUIRE(blocked.reachableSelf.size() < baseline.reachableSelf.size());
}
