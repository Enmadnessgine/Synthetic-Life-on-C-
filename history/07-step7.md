# Step 7: Final Ecosystem Simulation

## Changes Committed
- Rewrote `src/main.cpp` into a complete ecosystem simulation (Step 7 / Final Ecosystem)
- Fixed SFML 3.0 API compatibility: `pollEvent()` now returns `std::optional<sf::Event>`
- Fixed `Entity::OrganismType` scope qualification (nested enum class)
- Added `Entity::update()` call to the main loop (was missing — entities now move, bounce walls, metabolize)
- Removed duplicate `sf::Event event;` declaration
- Removed shadowed static reproduction timers; replaced with single non-static `asexualTimer`
- Removed unused sexual reproduction path (strategy classes exist but are not integrated into main loop)
- Added frame-rate-independent delta-time clamping (`dt` capped at 0.1s)
- Added per-second console statistics output (Flora / Herbivore / Predator counts)
- Flora photosynthesis energy gain increased to 25.f * dt with energy-based visual feedback
- Collision distances now use `genome.size` instead of `getRadius()`
- Reproduction threshold: `max(120.f, maxSpeed * 1.2f)`; parent passes 40% energy to child
- Child mutation: `maxSpeed ±10`, `size ±2`, random offset position/velocity
- Reproduction timer resets every frame (not just on success) to prevent retry spam

## Key Files Modified
| File | Change |
|------|--------|
| `src/main.cpp` | Full ecosystem loop rewrite (window, grid, physics, interactions, reproduction, render, stats) |
| `src/entities/Entity.h` | Added `OrganismType` enum, `getType()`, `setEnergy()`, `type_` member (from Step 4-6) |
| `src/entities/Entity.cpp` | Type-based constructor (radius/color/speed per type), metabolism `0.02f`, energy color feedback (from Step 4-6) |
| `src/spatial/SpatialGrid.h` | Added `cellSize` ctor param, `clear()`, `getNearbyEntitiesOfType()` (from Step 3-4) |
| `src/spatial/SpatialGrid.cpp` | Implementations for new ctor, `clear()`, `getNearbyEntitiesOfType()` (from Step 3-4) |

## Build
- CMake generator: Visual Studio 17 2022 (x64) — SFML 3.0 libs are MSVC-format, incompatible with MinGW
- Executable: `build/Debug/ArtificialLife.exe`
- Requires SFML DLLs next to `.exe`: `sfml-system-3.dll`, `sfml-window-3.dll`, `sfml-graphics-3.dll`

## Unused Code
- `src/reproduction/IReproductionStrategy.h`, `AsexualStrategy.h`, `SexualStrategy.h` exist but are not included in `CMakeLists.txt` or `main.cpp`; reproduction logic is inlined in `main.cpp`
- `SexualStrategy.h` redefines `IReproductionStrategy` and `AsexualStrategy` (duplicate class definitions)
