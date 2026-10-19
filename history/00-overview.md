# Project History: Synthetic Life on C

## Overview
C++ SFML project for Artificial Life simulation. Tracked evolution from basic SFML window to multi-entity spatial partitioning system.

---

## Step 1: Foundation (Initial Commit)
- **Commit:** Initial project setup
- **Files:** `CMakeLists.txt`, `src/main.cpp`, `test1.cpp`
- **Features:** 
  - C++17 CMake configuration
  - SFML 3.0.0 integration (Graphics, Window, System)
  - 1024x768 window with green bouncing circle
  - Delta time for frame-rate independent movement
- **Build:** `cmake --build build` → `ArtificialLife.exe`

## Step 2: Entity OOP Architecture
- **Commit:** Entity class refactoring
- **Files:** `src/entities\Entity.h`, `src/entities\Entity.cpp`, `src/main.cpp`
- **Features:**
  - Encapsulated Entity class with private `position`, `velocity`, `shape`
  - `update(dt, windowSize)` — position integration + wall bounce
  - `draw(target)` const — render shape to window
  - `std::vector<std::unique_ptr<Entity>>` container for dynamic entities
  - 4 entities with independent velocities
- **Build:** Same CMake build, Entity.cpp added to sources

## Step 3: SpatialGrid Architecture
- **Commit:** SpatialGrid class for spatial partitioning
- **Files:** `src/spatial/SpatialGrid.h`, `src/spatial/SpatialGrid.cpp`
- **Features:**
  - Grid with `CELL_SIZE = 100.f` resolution
  - `addEntity(position, index)` — insert entity into grid cell
  - `removeEntity(position, index)` — remove from cell
  - `updateEntity(oldPos, newPos, index)` — move entity to new cell
  - `getNearbyEntities(position)` — broadphase: 3×3 cell neighborhood query
  - Efficient entity tracking for scalable organism simulation
- **Integration:** CMakeLists.txt includes `SpatialGrid.cpp`; main.cpp ready for grid-based interaction

---

## Change Log Summary
| Step | Date | Focus |
|------|------|-------|
| 1 | 2026-10-06 | SFML window, game loop, delta time |
| 2 | 2026-10-09 | Entity class, OOP architecture, multiple entities |
| 3 | 2026-10-19 | SpatialGrid, spatial partitioning for scalability |

## Future Steps (Planned)
- Step 4: Reproduction strategies (asexual/sexual)
- Step 5: Genome/DNA traits inheritance
- Step 6: Collision detection and organism interactions