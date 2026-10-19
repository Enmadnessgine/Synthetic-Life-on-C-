# Step 1: Foundation

## Changes Committed
- Initial CMake project configuration with C++17
- SFML 3.0.0 integration (Graphics + Window + System components)
- Output executable: `ArtificialLife`
- `src/main.cpp`: SFML game loop with 1024x768 window, 60 FPS limit, green circle bouncing with delta time

## What Was Built
- **CMakeLists.txt**: `cmake_minimum_required(VERSION 3.14)`, `find_package(SFML 3.0 ...)`, `add_executable(ArtificialLife src/main.cpp)`
- **src/main.cpp**: Window creation, event handling (SFML 3.0 style), `sf::CircleShape` with `sf::Clock` for dt, wall bounce logic

## Runtime
- Requires SFML 3.0.0 DLLs: `sfml-graphics-3.dll`, `sfml-window-3.dll`, `sfml-system-3.dll`
- Or add `C:\SFML\SFML-3.0.0\bin` to system PATH

## Key Code Highlights
```cpp
// Delta time
float dt = clock.restart().asSeconds();
// Circle movement
circle.move(velocity * dt);
// Wall bounce
if (circle.getPosition().x <= 0 || circle.getRadius() * 2 >= size.x) velocity.x *= -1;
```

## Files Created/Modified
- `CMakeLists.txt` (new)
- `src/main.cpp` (new, replaced test1.cpp content)
- `.gitignore` (new, updated for build artifacts)
```
<tool_call>
<function=write>
<parameter=content>
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

## Runtime
- Same SFML DLL requirements as Step 1
- 4 green circles independently bouncing in 1024x768 window

## Files Created/Modified
- `src/entities\Entity.h` (new)
- `src/entities\Entity.cpp` (new)
- `src/main.cpp` (modified — replaced single circle with Entity vector)
- `CMakeLists.txt` (modified — added `src/entities/Entity.cpp`)
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

## Files Created/Modified
- `src/spatial/SpatialGrid.h` (new — 38 lines, grid declarations)
- `src/spatial/SpatialGrid.cpp` (new — 112 lines, grid implementation)
- `CMakeLists.txt` (modified — added `src/spatial/SpatialGrid.cpp` to executable)
- `src/main.cpp` (no code changes needed for grid basics; ready for grid integration)