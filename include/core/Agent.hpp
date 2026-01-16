#pragma once
#include "geom/Vec2.hpp"

struct Agent {
  Vec2 pos;
  Vec2 facing;   // should be unit; normalize on load
  double radius{0.25};
  double speed{5.0}; // units/sec
};
