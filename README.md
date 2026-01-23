# FPS Geometric Positioning Engine

A deterministic **C++ geometry analysis engine** for FPS-style positioning problems, paired with an **interactive raylib-based viewer** for experimentation and intuition-building.

This project focuses on explicit, explainable geometric reasoning about space, visibility, and exposure in short combat windows.

---

## Overview

The engine models simplified 2D FPS combat scenarios using axis-aligned geometry and computes interpretable positioning metrics. A standalone visual viewer allows real-time manipulation of agents and obstacles to explore how geometry affects advantage.

The core engine is headless, deterministic, and unit-tested. The viewer is intentionally separated and used only for visualization and interaction.

---

## Core Concepts

### Geometry Model
- World defined by axis-aligned bounding boxes (AABBs)
- Obstacles represented as rectangles
- Agents represented as circles with:
  - position
  - radius
  - maximum speed
  - facing direction (unit vector)

### Time Model
- Short fight window `T` (default: 0.30 seconds)
- Reachability radius computed as `speed × T`

---

## Analysis Metrics

The engine computes the following metrics per scene.

### Reachable Area Ratio
Compares how much space each agent can reach within the fight window, accounting for collisions with inflated obstacles.

This captures relative movement freedom.

---

### Exposure Width
Measures the lateral span of enemy positions that remain visible to the agent, projected onto the axis perpendicular to the agent’s facing direction.

This approximates how punishable a peek is from a geometric standpoint.

---

### Visible Hit Fraction
Samples points around the enemy’s hit circle and tests line-of-sight to each sample.

This estimates how much of the enemy is actually hittable from the agent’s current position and orientation.

---

### Explainability
Each analysis produces short explanation strings describing the mechanical and factual reasons behind the computed values.

---

## Interactive Viewer

The `fps_viewer` executable provides a real-time sandbox for exploring positioning geometry.

### Viewer Features
- Drag agents to reposition them
- Drag facing handles to rotate agent orientation
- Adjustable field-of-view angle (up to 360 degrees)
- Occlusion-aware FOV cones
- Create obstacles via click-drag
- Live recomputation of all metrics
- Visual overlays for reachability and visibility

### Viewer Controls
- Left mouse drag on agent: move agent
- Left mouse drag on facing handle: rotate facing direction
- Left mouse drag in empty space: create obstacle
- `[` / `]`: decrease / increase fight window `T`
- `,` / `.`: decrease / increase reachability grid size
- FOV slider: adjust field-of-view angle
- `1`: toggle self reachable area overlay
- `2`: toggle enemy reachable area overlay

---

## Repository Structure

.
├── include/
│ ├── analysis/ # Analysis interfaces and result types
│ ├── core/ # Scene, map, and agent definitions
│ └── geom/ # Vec2, AABB, raycasting utilities
├── src/
│ ├── analysis/ # Metric implementations
│ ├── core/
│ └── io/
├── viewer/
│ └── main.cpp # Interactive raylib viewer
├── examples/
│ ├── example_open.cpp
│ └── example_wall.cpp
├── tests/
│ ├── test_reachability.cpp
│ ├── test_exposure.cpp
│ ├── test_visibility.cpp
│ └── test_scene_analyzer.cpp
├── CMakeLists.txt
└── README.md


---

## Build Instructions

### Requirements
- CMake 3.20 or newer
- C++20-compatible compiler
- Git
- Windows: Visual Studio Build Tools
- Linux: GCC or Clang toolchain

---

### Build and Run (Windows)

```bash
cmake -S . -B build
cmake --build build --config Release -j
build/Release/fps_viewer.exe

Build and Run (Linux)

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/fps_viewer

Run Unit Tests

ctest --test-dir build --output-on-failure

Continuous Integration

GitHub Actions is used to:

    Build the project on Windows and Linux

    Run all unit tests on every push and pull request

Releases

Tagged versions (for example v0.1.0) automatically generate downloadable binaries via GitHub Releases, including the interactive viewer.
Design Philosophy

    Explicit geometry over implicit heuristics

    Deterministic, testable analysis

    Separation between analysis and visualization

    Minimal abstractions, introduced only when justified

License

MIT