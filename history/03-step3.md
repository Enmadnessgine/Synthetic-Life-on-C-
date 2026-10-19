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