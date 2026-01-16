#pragma once
#include "core/Map.hpp"
#include "core/Agent.hpp"

struct Scene {
  Map map;
  Agent self;
  Agent enemy;

  double T{0.30};
  double cellSize{0.5};
  int visibilitySamples{64};
};
