# Step 4: Reproduction Strategies

## Changes Committed
- Added reproduction strategy system with abstract base class `IReproductionStrategy`
- Implemented `AsexualStrategy` — clones parent with optional mutation
- Implemented `SexualStrategy` — combines traits from two parents (averaged position/velocity/color + mutation)
- Updated `src/entities\Entity.h` with getters/setters for reproduction access
- Updated `src/main.cpp` to include reproduction logic (timers, strategy calls, entity population growth)

## Strategy Design
- **`IReproductionStrategy`** — abstract base with 3 pure virtual functions:
  - `reproduce(parent, mutationRate)` — asexual
  - `reproduce(parent1, parent2, mutationRate)` — sexual
  - `shouldReproduce(entity, dt)` — timer-based trigger
- **`AsexualStrategy`** — clones parent entity, applies mutation to velocity/position/radius based on `mutationRate` probability
- **`SexualStrategy`** — creates offspring with averaged position/velocity from two parents, averaged color, + optional mutation

## Main Loop Changes
- Added reproduction timers (`asexualTimer`, `sexualTimer`) with `reproduceInterval = 5s` / `7s`
- `asexualStrategy->reproduce(*entities[parentIdx], 0.1f)` — creates clone with 10% mutation chance
- `sexualStrategy->reproduce(*entities[parent1Idx], *entities[parent2Idx], 0.1f)` — creates child from two parents
- New entities added to `std::vector<std::unique_ptr<Entity>>` container, population grows over time

## Key Additions
| File | Purpose |
|------|---------|
| `src/reproduction/IReproductionStrategy.h` | Abstract base class for all strategies |
| `src/reproduction/AsexualStrategy.h` | Clone parent + mutate velocity/position/radius |
| `src/reproduction/SexualStrategy.h` | Average two parents' traits + mutate |
| `src/entities\Entity.h` | Added getters: `getPosition()`, `getVelocity()`, `getRadius()`, `getColor()`; Setters: `setVelocity()`, `setPosition()`, `setRadius()`, `setColor()` |
| `src/main.cpp` | Reproduction timers + entity population growth logic |

## What Changed from Step 3
| Aspect | Step 3 | Step 4 |
|--------|--------|--------|
| **Entity Count** | Fixed 4 entities | Population grows via reproduction |
| **Strategy System** | N/A | `IReproductionStrategy` base + `AsexualStrategy`/`SexualStrategy` |
| **Entity Getters/Setters** | Basic access | Full access for reproduction system |
| **Main Loop** | Update/draw only | + reproduction timers + entity addition |

## Runtime
- Same SFML DLL requirements
- Starts with 4 entities, population grows over time (every 5s asexual, every 7s sexual)
- Each offspring has slight trait variations (mutation)

## Files Modified/Created
- `src/entities\Entity.h` (modified — added getters/setters)
- `src/main.cpp` (modified — reproduction logic)
- `src/reproduction/IReproductionStrategy.h` (new)
- `src/reproduction/AsexualStrategy.h` (new)
- `src/reproduction/SexualStrategy.h` (new)
- `CMakeLists.txt` (no change needed — new source files auto-detected)
- `history/04-step4.md` (new — this documentation)