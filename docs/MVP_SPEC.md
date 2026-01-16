# MVP Spec — FPS Geometric Positioning Engine (C++ / OOP)

## Goal
Deterministic C++ engine that takes 2D FPS-style geometry (AABB rectangles) + two agents and outputs explainable geometric advantage metrics.

## Potential progressions
 VOD ingestion,  abilities,  multi-agent,  UI requirement,  overall advantage score

## Inputs
- World bounds: width/height
- Obstacles: axis-aligned rectangles (AABB)
- Agents: position (x,y), radius r, max speed v, facing direction f (unit vector or angle)
- Fight window: T (default 0.30s)
- Sampling grid: cell_size

## Outputs (3 metrics)
A) Reachable Area Ratio
- Compute reachable cells within disk radius v*T, collision-constrained by inflated AABBs.

B) Exposure Width (empty-dome proxy)
- For enemy reachable cells with LoS from self, project onto axis perp(self_facing) and take width.

C) Visible Hit Fraction (model-dome proxy)
- Sample N points on target circle; LoS raycast to each; visible fraction.

## Geometry rules
- AABBs only
- Raycast blocks on obstacle intersection
- Reachability uses inflated AABBs (inflate by agent radius)

## Explainability
At least 2 explanation strings per run (mechanical, factual).

## Done criteria
- Builds on Linux via CMake
- `ctest` passes
- Two example scenes included and runnable
- README with build/run + sample output
