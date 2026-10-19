# Step 2: Entity OOP Architecture

## Changes Committed
- Refactored from hardcoded circle to Object-Oriented Entity class
- Added `src/entities\Entity.h` and `src/entities\Entity.cpp`
- Updated `src/main.cpp` to use `std::vector<std::unique_ptr<Entity>>`
- 4 entities with independent positions and velocities

## Entity Class Design
- **Encapsulation:** Private fields `position_`, `velocity_`, `shape_`
- **Constructor:** `Entity(sf::Vector2f position, sf::Vector2f velocity)` — initializer list, radius 20, green color, setPosition
- **update(float dt, const sf::Vector2u& windowSize):** `position_ += velocity_ * dt`; wall bounce: invert velocity.x when `position_.x <= 0` or `>= windowSize.x` (same for Y); `shape_.setPosition(position_)`
- **draw(sf::RenderTarget& target) const:** `target.draw(shape_)` — const-correct, no state modification

## Main Loop Changes
```cpp
std::vector<std::unique_ptr<Entity>> entities;
entities.push_back(std::make_unique<Entity>(sf::Vector2f(100.f, 100.f), sf::Vector2f(150.f, 100.f)));
entities.push_back(std::make_unique<Entity>(sf::Vector2f(500.f, 300.f), sf::Vector2f(-100.f, 50.f)));
entities.push_back(std::make_unique<Entity>(sf::Vector2f(300.f, 600.f), sf::Vector2f(200.f, -75.f)));
entities.push_back(std::make_unique<Entity>(sf::Vector2f(800.f, 200.f), sf::Vector2f(-150.f, -100.f)));

// In loop:
for (auto& entity : entities) entity->update(dt, window.getSize());
window.clear(sf::Color::Black);
for (const auto& entity : entities) entity->draw(window);
window.display();
```

## What Changed from Step 1
| Aspect | Step 1 | Step 2 |
|--------|--------|--------|
| **Code Structure** | Hardcoded `sf::CircleShape` in `main.cpp` | `Entity` class with `update()`/`draw()` methods |
| **Entity Management** | Single circle variable | `std::vector<std::unique_ptr<Entity>>` |
| **Bounce Logic** | Inline in `main.cpp` | Encapsulated in `Entity::update()` |
| **Const Correctness** | Not applicable | `draw()` marked `const` |

## Runtime
- 4 green circles independently bouncing in 1024x768 window
- Same SFML DLL requirements as Step 1

## Files Created/Modified
- `src/entities\Entity.h` (new — class declaration with private position/velocity/shape, public constructor/update/draw)
- `src/entities\Entity.cpp` (new — constructor with initializer list, update with wall bounce, draw const method)
- `src/main.cpp` (modified — replaced single circle with Entity vector of 4 entities)
- `CMakeLists.txt` (modified — added `src/entities/Entity.cpp` to executable)
```
<tool_call>
<function=write>
<parameter=content>
# Step 3: SpatialGrid Architecture

## Changes Committed
- Added spatial partitioning grid for efficient entity neighborhood detection
- Created `src/spatial/SpatialGrid.h` and `src/spatial/SpatialGrid.cpp`
- Updated `CMakeLists.txt` to include `src/spatial/SpatialGrid.cpp`
- Entity system (4 entities) remains active; Grid enhances scalability

## SpatialGrid Design
- **Cell Size:** `CELL_SIZE = 100.f` pixels — determines grid resolution
- **Grid Dimensions:** `ceil(worldWidth / CELL_SIZE) × ceil(worldHeight / CELL_SIZE)` = `11 × 8` for 1024×768 world
- **Storage:** Flat `std::vector<std::vector<int>> grid_` — 2D array of entity indices per cell
- **Cell Index Calculation:** `cellIndex = y * gridWidth_ + x` (row-major flat layout)

## Core Operations
```cpp
// Add entity to grid based on position
void addEntity(const sf::Vector2f& position, int entityIndex);

// Remove entity from grid
void removeEntity(const sf::Vector2f& position, int entityIndex);

// Move entity to new grid cell
void updateEntity(const sf::Vector2f& oldPos, const sf::Vector2f& newPos, int entityIndex);

// Get all entities in neighboring cells (broadphase)
std::vector<int> getNearbyEntities(const sf::Vector2f& position) const;
   // Checks 3×3 cell neighborhood (9 cells total)
   // Returns entity indices that could interact with given position
```

## Integration with Main Loop
SpatialGrid is now part of the project build. In the main loop, after entity updates:
```cpp
// Add entities to grid at start of frame
for (int i = 0; i < (int)entities.size(); i++) {
    grid.addEntity(entity->getPosition(), i);
}

// Query nearby entities for interaction
for (int i = 0; i < (int)entities.size(); i++) {
    auto neighbors = grid.getNearbyEntities(entity->getPosition());
    // ... perform collision/ interaction logic
}
```

## What Changed from Step 2
| Aspect | Step 2 | Step 3 |
|--------|--------|--------|
| **Code Structure** | Entity class with update/draw | + SpatialGrid for spatial partitioning |
| **Entity Tracking** | `std::vector<std::unique_ptr<Entity>>` only | + Grid cells track entity indices |
| **Neighbor Detection** | N/A (single-simulator mode) | `getNearbyEntities()` — 3×3 cell neighborhood |
| **Scalability** | Limited to few entities | Designed for many organisms |

## Files Created/Modified
- `src/spatial/SpatialGrid.h` (new — grid declarations: CELL_SIZE, grid dimensions, add/remove/update/getNearby)
- `src/spatial/SpatialGrid.cpp` (new — grid implementation: cell index calc, entity management, neighborhood query)
- `CMakeLists.txt` (modified — added `src/spatial/SpatialGrid.cpp` to executable)
- `src/main.cpp` (no code changes needed for grid basics; ready for grid integration)