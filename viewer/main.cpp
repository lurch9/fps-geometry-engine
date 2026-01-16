#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstring>
#include <limits>

#include "core/Scene.hpp"
#include "analysis/SceneAnalyzer.hpp"
#include "geom/AABB.hpp"
#include "geom/Vec2.hpp"

const int W = 1200;
const int H = 800;
static const int SIDEBAR_W = 360;
static const int VIEW_W = W - SIDEBAR_W;
static const int FOV_SLIDER_Y = 260;
static const int FOV_SLIDER_H = 16;


// ---------- World <-> Screen mapping ----------
struct Viewport {
  AABB world;
  int viewW = 840;
  int viewH = 800;
  int margin = 40;
  int x0 = 0;            // left offset of viewport in screen space
  double scale = 1.0;

  void computeScale() {
    const double worldW = world.max.x - world.min.x;
    const double worldH = world.max.y - world.min.y;
    const double sx = (viewW - 2.0 * margin) / worldW;
    const double sy = (viewH - 2.0 * margin) / worldH;
    scale = std::min(sx, sy);
  }

  bool mouseInView(const Vector2& m) const {
    return m.x >= x0 && m.x < (x0 + viewW) && m.y >= 0 && m.y < viewH;
  }

  Vector2 worldToScreen(const Vec2& p) const {
    const double x = x0 + margin + (p.x - world.min.x) * scale;
    const double y = margin + (world.max.y - p.y) * scale;
    return Vector2{(float)x, (float)y};
  }

  Vec2 screenToWorld(const Vector2& s) const {
    const double x = world.min.x + (s.x - x0 - margin) / scale;
    const double y = world.max.y - (s.y - margin) / scale;
    return Vec2{x, y};
  }

  Rectangle worldAABBToScreenRect(const AABB& b) const {
    Vector2 tl = worldToScreen(Vec2{b.min.x, b.max.y});
    Vector2 br = worldToScreen(Vec2{b.max.x, b.min.y});
    return Rectangle{tl.x, tl.y, br.x - tl.x, br.y - tl.y};
  }
};


static bool pointInCircle(const Vec2& p, const Vec2& c, double r) {
  const double dx = p.x - c.x;
  const double dy = p.y - c.y;
  return (dx*dx + dy*dy) <= (r*r);
}

static void drawFacingHandle(const Viewport& vp, const Agent& a, bool hovered) {
  const double handleDist = a.radius * 4.0;
  Vec2 f = a.facing.normalized();
  Vec2 handleW = a.pos + f * handleDist;
  Vector2 handleS = vp.worldToScreen(handleW);

  Color fill = hovered ? ORANGE : YELLOW;
  Color outline = hovered ? RED : ORANGE;

  DrawCircleV(handleS, 6.0f, fill);
  DrawCircleLines((int)handleS.x, (int)handleS.y, 6.0f, outline);
}

static Vec2 facingHandlePos(const Agent& a, double handleDist) {
  return a.pos + a.facing.normalized() * handleDist;
}

static void drawAgent(const Viewport& vp, const Agent& a, Color fill, Color outline) {
  Vector2 s = vp.worldToScreen(a.pos);
  const float rr = (float)(a.radius * vp.scale);

  DrawCircleV(s, rr, fill);
  DrawCircleLines((int)s.x, (int)s.y, rr, outline);

  // Facing arrow
  Vec2 f = a.facing.normalized();
  Vec2 tipW = a.pos + f * (a.radius * 3.0);
  Vector2 tipS = vp.worldToScreen(tipW);
  DrawLineEx(s, tipS, 2.0f, outline);
  DrawCircleV(tipS, 3.0f, outline);
  Vec2 handleW = facingHandlePos(a, a.radius * 4.0);
  Vector2 handleS = vp.worldToScreen(handleW);

  DrawCircleV(handleS, 6.0f, YELLOW);
  DrawCircleLines((int)handleS.x, (int)handleS.y, 6.0f, ORANGE);

}

static bool hitFacingHandle(const Vec2& mouseW, const Agent& a, double handleDist, double hitRadiusW) {
  Vec2 h = facingHandlePos(a, handleDist);
  return pointInCircle(mouseW, h, hitRadiusW);
}



static double raycastRayToAABB(const Vec2& ro, const Vec2& rdUnit, const AABB& b) {
  // Ray: ro + t*rd, t >= 0
  // Slab intersection, return nearest hit t, or INF if miss.
  const double INF = std::numeric_limits<double>::infinity();

  double tmin = 0.0;
  double tmax = INF;

  auto slab = [&](double roC, double rdC, double minC, double maxC) -> bool {
    if (std::abs(rdC) < 1e-12) {
      // Ray parallel to slab: must be inside
      return (roC >= minC && roC <= maxC);
    }
    double t1 = (minC - roC) / rdC;
    double t2 = (maxC - roC) / rdC;
    if (t1 > t2) std::swap(t1, t2);
    tmin = std::max(tmin, t1);
    tmax = std::min(tmax, t2);
    return tmin <= tmax;
  };

  if (!slab(ro.x, rdUnit.x, b.min.x, b.max.x)) return INF;
  if (!slab(ro.y, rdUnit.y, b.min.y, b.max.y)) return INF;

  // tmin is first intersection (could be 0 if starting inside)
  if (tmax < 0.0) return INF;       // box is behind ray
  if (tmin < 0.0) return 0.0;       // ray starts inside box; treat as immediate hit
  return tmin;
}

static double raycastToObstacles(const Map& map, const Vec2& origin, const Vec2& dirUnit, double maxRange) {
  double best = maxRange;

  for (const auto& ob : map.obstacles()) {
    double t = raycastRayToAABB(origin, dirUnit, ob);
    if (t >= 0.0 && t < best) best = t;
  }

  // Optional: clip to world bounds too (prevents cone drawing outside map)
  // Treat world bounds as a box you raycast against, but we want the INSIDE boundary.
  // Easiest: just clamp final points by maxRange and let the viewport/world show bounds.
  return best;
}

static void drawFovConeOccluded(
    const Viewport& vp,
    const Scene& scene,
    const Agent& a,
    double fovDeg,
    double maxRange,
    Color fill,
    Color edge)
{
  Vec2 f = a.facing.normalized();
  const double base = std::atan2(f.y, f.x);
  const double half = (fovDeg * (3.1415926535/ 180.0)) * 0.5;

  const int segments = 60; // higher = smoother + more accurate occlusion
  std::vector<Vec2> ptsW;
  ptsW.reserve(segments + 1);

  for (int i = 0; i <= segments; ++i) {
    const double t = (double)i / (double)segments;
    const double ang = base - half + (2.0 * half * t);

    Vec2 dir{ std::cos(ang), std::sin(ang) }; // already unit
    double d = raycastToObstacles(scene.map, a.pos, dir, maxRange);
    ptsW.push_back(a.pos + dir * d);
  }

  // Draw filled fan (triangle strip style)
  Vector2 cS = vp.worldToScreen(a.pos);
  for (int i = 0; i < segments; ++i) {
    Vector2 p0S = vp.worldToScreen(ptsW[i]);
    Vector2 p1S = vp.worldToScreen(ptsW[i + 1]);
    DrawTriangle(cS, p0S, p1S, fill);
  }

  // Outline edges
  DrawLineEx(cS, vp.worldToScreen(ptsW.front()), 2.0f, edge);
  DrawLineEx(cS, vp.worldToScreen(ptsW.back()), 2.0f, edge);

  // Outline along the clipped arc (optional but looks nice)
  for (int i = 0; i < segments; ++i) {
    DrawLineEx(vp.worldToScreen(ptsW[i]), vp.worldToScreen(ptsW[i + 1]), 1.0f, edge);
  }
}


static int DrawTextWrapped(const std::string& text, int x, int y, int fontSize, int maxWidth, Color color) {
  const int lineHeight = (int)(fontSize * 1.25f);

  std::string line;
  line.reserve(text.size());

  int yCursor = y;

  auto flushLine = [&]() {
    if (!line.empty()) {
      DrawText(line.c_str(), x, yCursor, fontSize, color);
      yCursor += lineHeight;
      line.clear();
    }
  };

  // Split on spaces and build lines
  size_t i = 0;
  while (i < text.size()) {
    // Skip leading spaces
    while (i < text.size() && text[i] == ' ') i++;
    if (i >= text.size()) break;

    // Read next word
    size_t start = i;
    while (i < text.size() && text[i] != ' ') i++;
    std::string word = text.substr(start, i - start);

    // Handle explicit newlines in the input (optional)
    if (word.find('\n') != std::string::npos) {
      // naive: split at '\n'
      size_t pos = 0;
      while (true) {
        size_t nl = word.find('\n', pos);
        std::string part = (nl == std::string::npos) ? word.substr(pos) : word.substr(pos, nl - pos);

        std::string candidate = line.empty() ? part : (line + " " + part);
        if (MeasureText(candidate.c_str(), fontSize) > maxWidth && !line.empty()) {
          flushLine();
          candidate = part;
        }
        line = candidate;

        if (nl == std::string::npos) break;
        flushLine();
        pos = nl + 1;
      }
      continue;
    }

    // Try to append word to current line
    std::string candidate = line.empty() ? word : (line + " " + word);

    if (MeasureText(candidate.c_str(), fontSize) <= maxWidth) {
      line = candidate;
    } else {
      // Current line full: flush and start new line with word
      flushLine();

      // If a single word is longer than maxWidth, hard-break it
      if (MeasureText(word.c_str(), fontSize) > maxWidth) {
        std::string chunk;
        for (char c : word) {
          std::string cand2 = chunk + c;
          if (MeasureText(cand2.c_str(), fontSize) <= maxWidth) {
            chunk = cand2;
          } else {
            DrawText(chunk.c_str(), x, yCursor, fontSize, color);
            yCursor += lineHeight;
            chunk = std::string(1, c);
          }
        }
        if (!chunk.empty()) {
          DrawText(chunk.c_str(), x, yCursor, fontSize, color);
          yCursor += lineHeight;
        }
      } else {
        line = word;
      }
    }
  }

  flushLine();

  // Return pixels consumed vertically
  return yCursor - y;
}



int main() {
  // --- Window ---
  InitWindow(W, H, "FPS Geometry Engine - raylib viewer");
  SetTargetFPS(60);

  // --- Scene (start with a reasonable default) ---
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

  // --- Viewport ---
  Viewport vp;
  vp.world = scene.map.worldBounds();
    vp.viewW = VIEW_W;  // left side only
    vp.viewH = H;
    vp.x0 = 0;
    vp.computeScale();


  

  // --- Analyzer state ---
  SceneAnalyzer analyzer;
  AnalysisResult result;
  bool dirty = true;

  // --- Interaction state ---
  enum class DragMode { None, Self, Enemy, SelfFacing, EnemyFacing };
  DragMode drag = DragMode::None;


  double fovDeg = 90.0;   // initial FOV angle

  bool showReachSelf = true;
  bool showReachEnemy = true;

  // Obstacle creation (click-drag in empty space)
  bool makingObstacle = false;
  Vec2 obsStartW;

  while (!WindowShouldClose()) {
    // Recompute analysis only when something changed
    if (dirty) {
      result = analyzer.analyze(scene);
      dirty = false;
    }

    // --- Input ---
    Vector2 mouseS = GetMousePosition();
    Vec2 mouseW = vp.screenToWorld(mouseS);

    // ---- FOV slider input ----
    static bool draggingFov = false;

    Rectangle fovTrack{
      (float)(VIEW_W + 16),
      (float)FOV_SLIDER_Y,
      (float)(SIDEBAR_W - 32),
      (float)FOV_SLIDER_H
    };

    bool mouseOnFov =
      CheckCollisionPointRec(mouseS, fovTrack);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOnFov) {
      draggingFov = true;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && draggingFov) {
      float t = (mouseS.x - fovTrack.x) / fovTrack.width;
      t = std::clamp(t, 0.0f, 1.0f);
      fovDeg = 5.0 + t * (360.0 - 5.0);
      dirty = true;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      draggingFov = false;
    }


    const double selfHandleDist  = scene.self.radius * 4.0;
    const double enemyHandleDist = scene.enemy.radius * 4.0;
    const double handleHitR      = scene.self.radius * 1.5;

    bool hoverSelfFacing =
    hitFacingHandle(mouseW, scene.self, selfHandleDist, handleHitR);

    bool hoverEnemyFacing =
    hitFacingHandle(mouseW, scene.enemy, enemyHandleDist, handleHitR);

    // Toggles
    if (IsKeyPressed(KEY_ONE)) { showReachSelf = !showReachSelf; }
    if (IsKeyPressed(KEY_TWO)) { showReachEnemy = !showReachEnemy; }

    // Simple T adjustments
    if (IsKeyPressed(KEY_LEFT_BRACKET)) { scene.T = std::max(0.05, scene.T - 0.05); dirty = true; }
    if (IsKeyPressed(KEY_RIGHT_BRACKET)) { scene.T = std::min(2.00, scene.T + 0.05); dirty = true; }

    // Simple cellSize adjustments
    if (IsKeyPressed(KEY_COMMA)) { scene.cellSize = std::max(0.10, scene.cellSize - 0.10); dirty = true; }
    if (IsKeyPressed(KEY_PERIOD)) { scene.cellSize = std::min(2.00, scene.cellSize + 0.10); dirty = true; }
    
    const bool inView = vp.mouseInView(mouseS);

// Drag with left mouse (handles > bodies > obstacle)
    if (inView && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      const double selfHandleDist = scene.self.radius * 4.0;
      const double enemyHandleDist = scene.enemy.radius * 4.0;
      const double handleHitR = std::max(scene.self.radius, scene.enemy.radius) * 1.5; // easy to grab

      // 1) Facing handles first
      if (hitFacingHandle(mouseW, scene.self, selfHandleDist, handleHitR)) {
        drag = DragMode::SelfFacing;
      } else if (hitFacingHandle(mouseW, scene.enemy, enemyHandleDist, handleHitR)) {
        drag = DragMode::EnemyFacing;

      // 2) Then agent bodies
      } else if (pointInCircle(mouseW, scene.self.pos, scene.self.radius * 1.8)) {
        drag = DragMode::Self;
      } else if (pointInCircle(mouseW, scene.enemy.pos, scene.enemy.radius * 1.8)) {
        drag = DragMode::Enemy;

      // 3) Otherwise start making a new obstacle
      } else {
        makingObstacle = true;
        obsStartW = mouseW;
      }
    }


    if (inView && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      if (drag == DragMode::Self) {
        scene.self.pos = mouseW;
        dirty = true;
      } else if (drag == DragMode::Enemy) {
        scene.enemy.pos = mouseW;
        dirty = true;
      } else if (drag == DragMode::SelfFacing) {
        Vec2 d = mouseW - scene.self.pos;
        if (d.norm() > 1e-6) { scene.self.facing = d.normalized(); dirty = true; }
      } else if (drag == DragMode::EnemyFacing) {
        Vec2 d = mouseW - scene.enemy.pos;
        if (d.norm() > 1e-6) { scene.enemy.facing = d.normalized(); dirty = true; }
      }
    }


    if (inView && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      if (makingObstacle) {
        makingObstacle = false;

        Vec2 a = obsStartW;
        Vec2 b = mouseW;

        // normalize min/max
        AABB ob;
        ob.min = Vec2{ std::min(a.x, b.x), std::min(a.y, b.y) };
        ob.max = Vec2{ std::max(a.x, b.x), std::max(a.y, b.y) };

        // Ignore tiny drags
        if ((ob.max.x - ob.min.x) > 0.1 && (ob.max.y - ob.min.y) > 0.1) {
          scene.map.addObstacle(ob);
          dirty = true;
        }
      }
      drag = DragMode::None;
    }

    // --- Draw ---
    BeginDrawing();
    ClearBackground(RAYWHITE);
    Rectangle sidebar{ (float)VIEW_W, 0, (float)SIDEBAR_W, (float)H };
    DrawRectangleRec(sidebar, Color{245,245,245,255});
    DrawRectangleLinesEx(sidebar, 1, LIGHTGRAY);


    // World bounds
    DrawRectangleLinesEx(vp.worldAABBToScreenRect(scene.map.worldBounds()), 2, DARKGRAY);

    // Obstacles
    for (const auto& ob : scene.map.obstacles()) {
      Rectangle r = vp.worldAABBToScreenRect(ob);
      DrawRectangleRec(r, Color{60, 60, 60, 255});
      DrawRectangleLinesEx(r, 1, BLACK);
    }

    // Preview obstacle while creating
    if (makingObstacle) {
      AABB preview;
      preview.min = Vec2{ std::min(obsStartW.x, mouseW.x), std::min(obsStartW.y, mouseW.y) };
      preview.max = Vec2{ std::max(obsStartW.x, mouseW.x), std::max(obsStartW.y, mouseW.y) };
      Rectangle r = vp.worldAABBToScreenRect(preview);
      DrawRectangleLinesEx(r, 2, RED);
    }

    // Reachable points
    const float ptR = 2.0f;
    if (showReachSelf) {
      for (const auto& p : result.reachability.reachableSelf) {
        Vector2 s = vp.worldToScreen(p);
        DrawCircleV(s, ptR, Color{0, 120, 255, 140});
      }
    }
    if (showReachEnemy) {
      for (const auto& p : result.reachability.reachableEnemy) {
        Vector2 s = vp.worldToScreen(p);
        DrawCircleV(s, ptR, Color{255, 60, 60, 120});
      }
    }

    drawFovConeOccluded(
      vp,
      scene,
      scene.self,
      fovDeg,
      8.0,
      Color{0,140,255,35},
      Color{0,140,255,120}
    );

    drawFovConeOccluded(
      vp,
      scene,
      scene.enemy,
      fovDeg,
      8.0,
      Color{255,80,80,30},
      Color{255,80,80,120}
    );


    // Agents
    drawAgent(vp, scene.self, Color{0, 140, 255, 220}, BLUE);
    drawAgent(vp, scene.enemy, Color{255, 80, 80, 220}, MAROON);
    drawFacingHandle(vp, scene.self, hoverSelfFacing);
    drawFacingHandle(vp, scene.enemy, hoverEnemyFacing);


    // HUD
    int x = VIEW_W + 16;
    int y = 20;

    DrawText("Controls:", x, y, 18, BLACK); y += 22;
    DrawText("  Drag agents with LMB", x, y, 16, DARKGRAY); y += 18;
    DrawText("  LMB drag empty space: create obstacle", x, y, 16, DARKGRAY); y += 18;
    DrawText("  [ / ] : T down/up", x, y, 16, DARKGRAY); y += 18;
    DrawText("  , / . : cellSize down/up", x, y, 16, DARKGRAY); y += 18;
    DrawText("  1: toggle self reachable", x, y, 16, DARKGRAY); y += 18;
    DrawText("  2: toggle enemy reachable", x, y, 16, DARKGRAY); y += 26;
    y = std::max(y, FOV_SLIDER_Y + 60);
    // ---- FOV slider UI ----
    DrawText("Field of View (deg)", VIEW_W + 16, FOV_SLIDER_Y - 22, 16, BLACK);

    // Track
    DrawRectangleRec(fovTrack, LIGHTGRAY);
    DrawRectangleLinesEx(fovTrack, 1, DARKGRAY);

    // Knob
    float knobT = (float)((fovDeg - 5.0) / (360.0 - 5.0));
    float knobX = fovTrack.x + knobT * fovTrack.width;

    DrawCircle((int)knobX, (int)(fovTrack.y + fovTrack.height / 2), 7, DARKBLUE);

    // Value label
    DrawText(TextFormat("%.0f°", fovDeg),
            VIEW_W + 16,
            FOV_SLIDER_Y + 24,
            16,
            DARKGRAY);



    {
      std::string line = "T=" + std::to_string(scene.T) + "  cellSize=" + std::to_string(scene.cellSize) +
                         "  visSamples=" + std::to_string(scene.visibilitySamples);
      DrawText(line.c_str(), x, y, 18, BLACK); y += 24;
    }
    {
      std::string line = "Reachable Area Ratio: " + std::to_string(result.reachability.areaRatio);
      DrawText(line.c_str(), x, y, 18, BLACK); y += 22;
    }
    {
      std::string line = "Exposure Width: " + std::to_string(result.exposure.width);
      DrawText(line.c_str(), x, y, 18, BLACK); y += 22;
    }
    {
      std::string line = "Visible Hit Fraction: " + std::to_string(result.visibility.visibleFraction);
      DrawText(line.c_str(), x, y, 18, BLACK); y += 22;
    }

    // Explanation strings (first 2)
// Explanation strings (wrap inside sidebar)
    y += 10;

    const float wrapW = (float)SIDEBAR_W - 32.0f;   // sidebar padding (16 left + 16 right)
    const float fontSize = 14.0f;
    const float spacing  = 1.0f;

    for (size_t i = 0; i < result.explanations.size() && i < 2; ++i) {
      Rectangle r{ (float)x, (float)y, wrapW, 200.0f }; // 200px tall "box" for wrapped text
      DrawTextWrapped(result.explanations[i].c_str(), x, y, fontSize, wrapW, GRAY);

      // advance y by the box height + padding (simple + reliable for now)
      y += (int)r.height + 10;
    }


    EndDrawing();
  }

  CloseWindow();
  return 0;
}
