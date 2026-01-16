#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstring>

#include "analysis/SceneAnalyzer.hpp"
#include "geom/AABB.hpp"
#include "geom/Vec2.hpp"

const int W = 1200;
const int H = 800;
static const int SIDEBAR_W = 360;
static const int VIEW_W = W - SIDEBAR_W;

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
  enum class DragMode { None, Self, Enemy };
  DragMode drag = DragMode::None;

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

    // Drag self/enemy with left mouse
    if (inView && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      // Prefer grabbing agents first
      if (pointInCircle(mouseW, scene.self.pos, scene.self.radius * 1.8)) {
        drag = DragMode::Self;
      } else if (pointInCircle(mouseW, scene.enemy.pos, scene.enemy.radius * 1.8)) {
        drag = DragMode::Enemy;
      } else {
        // Start making a new obstacle
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

    // Agents
    drawAgent(vp, scene.self, Color{0, 140, 255, 220}, BLUE);
    drawAgent(vp, scene.enemy, Color{255, 80, 80, 220}, MAROON);

    // HUD
    int x = VIEW_W + 16, y = 20;
    DrawText("Controls:", x, y, 18, BLACK); y += 22;
    DrawText("  Drag agents with LMB", x, y, 16, DARKGRAY); y += 18;
    DrawText("  LMB drag empty space: create obstacle", x, y, 16, DARKGRAY); y += 18;
    DrawText("  [ / ] : T down/up", x, y, 16, DARKGRAY); y += 18;
    DrawText("  , / . : cellSize down/up", x, y, 16, DARKGRAY); y += 18;
    DrawText("  1: toggle self reachable", x, y, 16, DARKGRAY); y += 18;
    DrawText("  2: toggle enemy reachable", x, y, 16, DARKGRAY); y += 26;


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
