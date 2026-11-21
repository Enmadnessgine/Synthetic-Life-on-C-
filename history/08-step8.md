# Step 8: Native SFML UI Integration & Input Handling

## Overview
Implemented a complete native SFML-based UI system for the ArtificialLife ecosystem simulation. Replaced the failed ImGui-SFML integration attempt with a custom native SFML UI built using `sf::RectangleShape`, `sf::Text`, and `sf::CircleShape`.

## Changes Committed

### 8.1: Native SFML UI Panels (Step 8.1)
- **Complete UI System** built with native SFML primitives (`sf::RectangleShape`, `sf::Text`, `sf::VertexArray`)
- **No external dependencies** — pure SFML 3.0, no ImGui dependency
- **Layered rendering**: Entities → UI Panels → Menu Bar (top-most)

#### UI Panels Implemented
| Panel | Position | Size | Content |
|-------|----------|------|---------|
| **Menu Bar** | Top (0,0) | 1024×25 | Title, pause status, time scale, logging status |
| **Control Panel** | Left Top (10,40) | 280×520 | Time scale, pause, population settings, metabolism, vision ranges |
| **Population Stats** | Right Top (740,40) | 270×200 | Real-time counts, avg energy, total energy |
| **Experiments** | Right Middle (740,250) | 270×300 | Add/kill entities, reset, CSV export, logging toggle |
| **Graphs** | Bottom (10,570) | 1000×180 | Population history lines (Flora/Herbivore/Predator) |
| **Logging** | Bottom Bar (10,760) | 1000×40 | CSV logging status |

#### Visual Design
- **Semi-transparent backgrounds**: `sf::Color(40,40,50,230)` — entities visible underneath
- **Color-coded entities**: Flora (green), Herbivore (gold), Predator (red)
- **Hover effects**: Buttons brighten on mouse hover
- **Font**: System Arial/Segoe UI with fallback

### 8.2: Complete Input Handling (Step 8.2)

#### Mouse Click Handling
Implemented `sf::Event::MouseButtonPressed` handling with precise `sf::FloatRect` hit-testing:

| Button | Action |
|--------|--------|
| **Add 50 Flora** | Spawns 50 Flora at random positions with 200 energy |
| **Add 20 Herbivore** | Spawns 20 Herbivores with 250 energy, random velocity |
| **Add 5 Predator** | Spawns 5 Predators with 300 energy, random velocity |
| **Kill 50% Predators** | Randomly kills half the predator population |
| **Kill 50% Herbivores** | Randomly kills half the herbivore population |
| **Kill All Flora** | Instantly kills all flora |
| **Reset Simulation** | Full population reset with history clear |
| **Export State (CSV)** | Exports `simulation_state.csv` with entity data |
| **Hide/Show HUD [H]** | Toggles all UI panels visibility |
| **Toggle CSV Logging** | Enables/disables `simulation_log.csv` |

#### Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `P` | Toggle pause/resume |
| `+` / `-` | Increase/decrease time scale (0.1x - 3.0x) |
| `L` | Toggle CSV logging |
| `R` | Reset simulation (full population reset) |
| `H` | Toggle HUD visibility |

#### Hover Effects
- Buttons brighten on mouse hover (visual feedback)
- Implemented via `makeRect()` helper and `sf::FloatRect::contains()`
- Hover detection uses real-time mouse position from `sf::Mouse::getPosition()`

### 8.3: HUD Toggle [H] (Step 8.3)
- **New field**: `bool showUI` in `UIState` struct (default `true`)
- **Keyboard**: `H` key toggles visibility
- **Button**: "Hide HUD [H]" / "Show HUD [H]" in Experiments panel
- **Behavior**: When hidden, all panels hidden but entities still render and simulation continues
- **Menu bar** remains visible even when HUD is hidden

### 8.4: Input Handling Architecture (Step 8.4)

#### SFML 3.0 Event Loop
```cpp
while (std::optional<sf::Event> event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) { window.close(); }
    else if (event->is<sf::Event::KeyPressed>()) { /* keyboard shortcuts */ }
    else if (event->is<sf::Event::MouseButtonPressed>()) { /* click handling */ }
}
```

#### Button Hit Detection
```cpp
sf::FloatRect btnAddFlora = makeRect(750.f, 280.f, 200.f, 25.f);
if (btnAddFlora.contains(mousePos)) { /* spawn 50 flora */ }
```

#### Helper: `makeRect()`
```cpp
inline sf::FloatRect makeRect(float left, float top, float width, float height) {
    sf::FloatRect rect;
    rect.position = {left, top};
    rect.size = {width, height};
    return rect;
}
```
SFML 3.0 `sf::FloatRect` uses `position` and `size` members, not `left/top/width/height`.

### 8.5: UI State Management
Added `UIState` struct with:
```cpp
struct UIState {
    sf::Font font;
    bool fontLoaded = false;
    bool showUI = true;                    // HUD toggle
    bool showPopulationStats = true;
    bool showControlPanel = true;
    bool showGraphsPanel = true;
    bool showExperimentPanel = true;
    bool showLoggingPanel = true;
    bool isLogging = false;
    
    std::vector<float> floraHistory;
    std::vector<float> herbivoreHistory;
    std::vector<float> predatorHistory;
    int maxHistoryPoints = 600;
    std::ofstream logFile;
    float logTimer = 0.0f;
    
    SimParams params;
};
```

### 8.6: CSV Logging & Export
- **Auto-logging**: Every second writes `Time,Flora,Herbivore,Predator` to `simulation_log.csv`
- **Manual export**: "Export State (CSV)" button writes `simulation_state.csv` with full entity data:
  ```
  Type,PositionX,PositionY,VelocityX,VelocityY,Energy,MaxSpeed,Size
  ```

### 8.5: Performance & Rendering
- **Layered rendering**: Flora → Herbivores → Predators → UI Panels → Menu Bar
- **Semi-transparent panels**: `sf::Color(40,40,50,230)` — entities visible underneath
- **Menu bar**: Drawn last (top-most) with solid background
- **History buffer**: 600 points per population type (~10 minutes at 1 FPS)

### 8.6: Known Limitations
- **No slider interaction**: Sliders are visual only
- **No text input**: Population values adjustable via keyboard only
- **Button rectangles**: Static coordinates; window resize not handled
- **No drag/drop**: Buttons only respond to clicks
- **No tooltip system**: Hover only changes button color

## Files Modified
| File | Changes |
|-------|---------|
| `src/main.cpp` | Complete UI system, input handling, rendering |
| `src/entities/Entity.h` | FSM enums, state fields, getters/setters |
| `src/entities/Entity.cpp` | FSM state transitions, metabolism, visual feedback |
| `CMakeLists.txt` | Removed ImGui dependencies |

## Build Configuration
- **CMake**: Visual Studio 17 2022 (x64), C++17
- **SFML 3.0**: Graphics, Window, System components
- **No ImGui**: Native SFML only
- **Debug DLLs**: `sfml-system-d-3.dll`, `sfml-window-d-3.dll`, `sfml-graphics-d-3.dll` in `build/Debug/`

## Key Technical Decisions
1. **Removed ImGui**: Incompatible with SFML 3.0 (API changes, missing internal headers)
2. **Native SFML UI**: Simpler, fewer dependencies, full control
3. **Aggregate initialization**: `sf::FloatRect` requires `makeRect()` helper (SFML 3.0 uses `position`/`size`)
4. **SFML 3.0 API**: `pollEvent()` returns `std::optional`, `sf::Font::openFromFile()`, `sf::Keyboard::Key::Hyphen`

## Expected Behavior
- All buttons clickable with hover feedback
- Keyboard shortcuts functional
- HUD toggle (`H` key) hides/shows all panels
- CSV logging to `simulation_log.csv` every second
- Export creates `simulation_state.csv`
- Reset (`R`) fully reinitializes population
- Console stats every second: `Flora: X | Herbivore: Y | Predator: Z`

## Build & Run
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
Copy-Item "C:/SFML/SFML-3.0.0/bin/sfml-*-d-3.dll" -Destination "build/Debug/"
build/Debug/ArtificialLife.exe
```