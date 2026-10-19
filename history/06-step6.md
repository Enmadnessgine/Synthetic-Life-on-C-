# Step 6: Energy, Metabolism & Natural Selection

## Changes Complemented
- Added `energy_` field to Entity class with metabolism deduction per frame
- Added `getEnergy()`, `addEnergy()`, `isDead()` methods to Entity
- Updated `Entity::update()` to deduct metabolism cost (`maxSpeed * 0.1f * dt`)
- Updated `Entity::draw()` to provide visual feedback via alpha channel and color shift
- Updated reproduction strategies: reproduction only occurs when `energy >= maxSpeed * 2.0f`
- Parent loses 50% energy on reproduction, child inherits the energy state
- Implemented "Mark and Sweep" pattern using `std::remove_if` + `std::erase` (C++17) for safe entity cleanup
- Population now self-regulates: entities die of starvation when energy reaches 0

## Key Changes

### Entity Class (`src/entities/Entity.h`)
- Added `float energy_ = 100.f` member field
- Added `getEnergy()`, `addEnergy(float)`, `isDead()` methods
- `update()` deducts `metabolism_cost * dt` where `metabolism_cost = genome_.maxSpeed * 0.1f`
- `draw()` provides visual feedback:
  - Energy >= 30%: green circle with alpha proportional to energy
  - Energy < 30%: red circle with alpha proportional to energy

### Reproduction Strategies
- **AsexualStrategy::shouldReproduce()**: checks `energy >= genome.maxSpeed * 2.0f`
- **SexualStrategy::shouldReproduce()**: checks both parents have sufficient energy
- On successful reproduction: parent loses 50% of current energy
- Reproduction timers still exist but are now secondary to energy check

### Main Loop (`src/main.cpp`)
- Added `std::remove_if` + `std::erase` pattern (C++17) for safe entity cleanup:
  ```cpp
  entities.erase(
      std::remove_if(entities.begin(), entities.end(), 
          [](const std::unique_ptr<Entity>& e) { return e->isDead(); }),
      entities.end()
  );
  ```
- Reproduction logic now checks energy thresholds before creating offspring
- Energy cost (50% loss) applied to parent on successful reproduction

### Expected Behavior
- **Without energy**: Population grows exponentially (previous behavior)
- **With energy**: Population self-regulates naturally
  - Fast entities (high maxSpeed) metabolize faster → die younger
  - Slow entities survive longer
  - Reproduction requires energy reserve → limits overpopulation
  - Visual feedback: alpha/color shows energy state at a glance
  - Equilibrium population stabilizes based on metabolism rates

### Files Modified
- `src/entities/Entity.h` — added energy field, getters/setters, metabolism in update, visual feedback in draw
- `src/entities/Entity.cpp` — implemented energy deduction and visual alpha feedback
- `src/reproduction/AsexualStrategy.h` — energy check in shouldReproduce
- `src/reproduction/SexualStrategy.h` — energy check in shouldReproduce
- `src/main.cpp` — std::remove_if for entity cleanup, energy-based reproduction logic, energy cost on reproduction

### History Documentation
- `history/06-step6.md` — detailed step log

## Change Summary Table

| Aspect | Before Step 6 | After Step 6 |
|--------|--------------|--------------|
| **Energy** | None — entities never die | Energy field, metabolism, death by starvation |
| **Reproduction** | Timer-based, unlimited growth | Energy-threshold-based, self-limiting |
| **Population** | Grows indefinitely | Self-regulates at equilibrium |
| **Visual Feedback** | None | Alpha/color shows energy level |
| **Entity Cleanup** | Not implemented (risk of undefined behavior) | `std::remove_if` + `std::erase` pattern |

## Future Steps (Planned)
- Step 7: Enhanced collision detection and organism interactions
- Step 8: Genetic algorithm optimization
- Step 9: Spatial partitioning with energy-aware culling