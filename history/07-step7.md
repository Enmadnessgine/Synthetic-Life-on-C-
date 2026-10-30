# Step 7: Final Ecosystem Simulation (Balanced v2)

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
- Collision distances now use `genome.size` instead of `getRadius()`
- Child mutation: `maxSpeed ±10`, `size ±2`, random offset position/velocity
- Reproduction timer resets every frame (not just on success) to prevent retry spam

## Ecosystem Balancing (Step 7.2)
- **Population limits:** `MAX_FLORA=1000`, `MAX_HERBIVORE=100`, `MAX_PREDATOR=15` (пропорція ~100:10:1 енергетичної піраміди)
- **Reduced initial population:** Flora 150→100, Herbivore 30→20, Predator 10→8
- **Population counting** before interaction loop each frame (per-type tracking with newborn counts to prevent overshoot)
- **Photosynthesis:** rate 25→18 `* dt`, disabled when `floraCount >= MAX_FLORA`
- **Reproduction timer:** 6→4 seconds (флора розмножується частіше)
- **Reproduction limits:** blocked per-type when population reaches cap
- **Reproduction threshold (predators):** `max(300.f, maxSpeed * 2.5f)` (вищий поріг для хижаків vs `max(120.f, maxSpeed * 1.2f)` для інших)
- **Energy from eating:** Herbivore→Flora 60→80; Predator→Herbivore 120→80 (менше енергії на вищому трофічному рівні — правило 10%)
- **Metabolism (Entity.cpp):** base `0.015f`, predators `0.025f` (хижаки витрачають більше енергії на обмін речовин)

## Біологічний сенс (для пояснення викладачу)
- **Енергетична піраміда:** на кожному трофічному рівні втрачається ~90% енергії (правило 10%).
  Тому популяція стабілізується у пропорції приблизно 100:10:1 (Флора:Травоїдні:Хижаки).
- **Голодова смерть:** істоми, енергія яких вичерпана до 0, видаляються — природний механізм регулювання.
- **Вищий трофічний рівень = вищий поріг розмноження та вища метаболізм-вартість.**

## Key Files Modified
| File | Change |
|------|--------|
| `src/main.cpp` | Full ecosystem loop rewrite + balancing: population limits, counting, capped photosynthesis, capped reproduction, type-specific eating energy & thresholds |
| `src/entities/Entity.h` | Added `OrganismType` enum, `getType()`, `setEnergy()`, `type_` member |
| `src/entities/Entity.cpp` | Type-based constructor, type-aware metabolism (base 0.015f, predator 0.025f), energy color feedback |
| `src/spatial/SpatialGrid.h` | Added `cellSize` ctor param, `clear()`, `getNearbyEntitiesOfType()` |
| `src/spatial/SpatialGrid.cpp` | Implementations for new ctor, `clear()`, `getNearbyEntitiesOfType()` |

## Build
- CMake generator: Visual Studio 17 2022 (x64) — SFML 3.0 libs are MSVC-format, incompatible with MinGW
- Executable: `build/Debug/ArtificialLife.exe`
- Requires SFML DLLs next to `.exe`: `sfml-system-3.dll`, `sfml-window-3.dll`, `sfml-graphics-3.dll`

## Unused Code
- `src/reproduction/IReproductionStrategy.h`, `AsexualStrategy.h`, `SexualStrategy.h` exist but are not included in `CMakeLists.txt` or `main.cpp`; reproduction logic is inlined in `main.cpp`
- `SexualStrategy.h` redefines `IReproductionStrategy` and `AsexualStrategy` (duplicate class definitions)