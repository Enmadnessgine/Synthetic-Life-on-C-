# Artificial Life — C++ SFML Ecosystem Simulation

A biologically-inspired ecosystem simulation built with C++17 and SFML 3.0, featuring a complete food chain with Flora, Herbivores, and Predators governed by Finite State Machines, energy metabolism, and evolutionary dynamics.

![Ecosystem](https://img.shields.io/badge/Ecosystem-Complete-green)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![SFML](https://img.shields.io/badge/SFML-3.0-green)
![License](https://img.shields.io/badge/License-MIT-blue)

## 🎬 Demo

```
=== Initializing FSM Ecosystem with UI ===
Created entities. Total: 148

Stats -> Flora: 69 | Herbivore: 37 | Predator: 8
Stats -> Flora: 59 | Herbivore: 37 | Predator: 8
Stats -> Flora: 86 | Herbivore: 69 | Predator: 15
Stats -> Flora: 173 | Herbivore: 100 | Predator: 15
...
```

---

## 🎯 Features

### 🌿 Complete Ecosystem Simulation
- **Three trophic levels**: Flora (producers) → Herbivores (primary consumers) → Predators (secondary consumers)
- **Energy pyramid**: ~90% energy loss per trophic level (Rule of 10%)
- **Population balance**: Stabilizes at ~100:10:1 ratio (Flora:Herbivore:Predator)

### 🧠 Finite State Machine Behavior
| Entity | States | Transitions |
|--------|--------|-------------|
| **Herbivore** | SEEK_FOOD, FLEE, SEEK_MATE, WANDER | Energy-based + predator proximity |
| **Predator** | SEEK_PREY, WANDER | Prey detection + energy |
| **Flora** | PHOTOSYNTHESIS | Energy accumulation |

**Priority Logic**: FLEE > SEEK_MATE > SEEK_FOOD > WANDER (Herbivore) | SEEK_PREY > WANDER (Predator)

### 🌍 World & Physics
- **Toroidal world**: Pac-Man style wrap-around (entities exiting edges reappear on opposite side)
- **Toroidal SpatialGrid**: Efficient O(1) neighbor queries via uniform grid partitioning
- **Lerp smoothing**: Velocity interpolation (0.15 factor) prevents jittering
- **Delta-time clamping**: Max 0.1s/frame prevents lag spikes

### 🧬 Genetics & Evolution
- **Inheritable traits**: maxSpeed, size, color, visionRange, mutationRate
- **Asexual reproduction**: Cloning with mutation (±10 speed, ±2 size)
- **Mutation rate**: Heritable, mutates during reproduction
- **Energy-based reproduction**: Thresholds based on maxSpeed and base energy

### 🎮 Native SFML UI (No ImGui)
| Panel | Features |
|-------|----------|
| **Control Panel** | Time scale (0.1x-3x), Pause (P), Population settings, Metabolism, Vision ranges |
| **Population Stats** | Real-time counts, Average energy per type, Total energy |
| **Population Graphs** | Real-time line charts (Flora/Herbivore/Predator) |
| **Experiments** | Add/Kill entities, Reset, CSV Export, Logging toggle |
| **Population History** | 600-point buffer (10 min at 1 FPS) |

### ⌨️ Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `P` | Pause/Resume |
| `+` / `-` | Time scale (0.1x - 3.0x) |
| `L` | Toggle CSV logging |
| `R` | Reset simulation (full reset) |
| `H` | Toggle HUD visibility |

### 📊 Data Export
- **Auto-logging**: `simulation_log.csv` (every second: Time,Flora,Herbivore,Predator)
- **Manual export**: `simulation_state.csv` with full entity data
- **Logging toggle**: L key or UI button

---

## 🏗 Architecture

```
src/
├── main.cpp                 # Main simulation loop, UI, input handling
├── entities/
│   ├── Entity.h            # Entity class, FSM enums, Genome struct
│   ├── Entity.cpp          # FSM logic, metabolism, physics, rendering
│   └── genome.h            # Genome struct (genetic traits)
├── spatial/
│   ├── SpatialGrid.h       # Uniform grid spatial partitioning
│   └── SpatialGrid.cpp     # Grid operations, neighbor queries
└── reproduction/           # Unused strategy pattern (legacy)
    ├── IReproductionStrategy.h
    ├── AsexualStrategy.h
    └── SexualStrategy.h
```

### Core Classes

**Entity** — Main simulation entity with:
- Position, velocity, energy, genome, type
- FSM states (HerbivoreState, PredatorState)
- Physics: position += velocity * dt, toroidal wrap, lerp velocity
- Metabolism: energy drain proportional to maxSpeed
- Visual feedback: alpha modulation based on energy

**SpatialGrid** — Uniform grid for O(1) neighbor queries:
- Cell size: 100px (11×8 cells for 1024×768 world)
- Methods: `addEntity()`, `clear()`, `getNearbyEntitiesOfType()`

**Genome** — Inheritable traits:
- `maxSpeed`, `size`, `color`, `visionRange`, `mutationRate`
- Mutates during reproduction (±10 speed, ±2 size)

---

## 🚀 Quick Start

### Prerequisites
- **CMake** 3.14+
- **Visual Studio 2022** (Community/Professional/Enterprise) with C++ workload
- **SFML 3.0.0** installed at `C:/SFML/SFML-3.0.0/`

### Build Instructions

```powershell
# Clone
git clone <repository-url>
cd ArtificialLife

# Configure (Visual Studio 2022 x64)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Build (Debug or Release)
cmake --build build --config Debug    # For debugging
cmake --build build --config Release  # For distribution

# Copy SFML DLLs (required for execution)
Copy-Item "C:/SFML/SFML-3.0.0/bin/sfml-*-d-3.dll" -Destination "build/Debug/"   # Debug
Copy-Item "C:/SFML/SFML-3.0.0/bin/sfml-*-3.dll" -Destination "build/Release/"  # Release

# Run
build/Debug/ArtificialLife.exe   # Debug
build/Release/ArtificialLife.exe # Release (optimized)
```

### Release Build for Distribution
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Copy Release DLLs: sfml-system-3.dll, sfml-window-3.dll, sfml-graphics-3.dll
```

---

## 🎮 Controls Reference

| Input | Action |
|-------|--------|
| **Mouse Click** | Click buttons in Experiments panel |
| **Hover** | Buttons brighten on mouse hover |
| `P` | Pause / Resume |
| `+` / `=` | Increase time scale (+0.1x) |
| `-` / `_` | Decrease time scale (-0.1x) |
| `P` | Pause / Resume |
| `L` | Toggle CSV logging |
| `R` | Reset simulation (full population reset) |
| `H` | Toggle HUD visibility |
| Window X | Exit application |

---

## 📊 CSV Output Format

### Auto-log (`simulation_log.csv`)
```csv
Time,Flora,Herbivore,Predator
0,100,40,8
1,102,41,8
2,105,42,8
...
```

### Manual Export (`simulation_state.csv`)
```csv
Type,PositionX,PositionY,VelocityX,VelocityY,Energy,MaxSpeed,Size
0,512.5,384.0,0.0,0.0,150.0,0.0,9.0
1,123.4,567.8,12.5,-8.3,200.0,220.0,12.0
...
```
Types: 0=Flora, 1=Herbivore, 2=Predator

---

## 📈 Expected Stable State

After ~60 seconds, populations stabilize at:
| Population | Target Range | Notes |
|------------|--------------|-------|
| **Flora** | 800-1000 | Base of energy pyramid |
| **Herbivore** | 60-80 | Primary consumers |
| **Predator** | 10-15 | Top predators |

---

## 🔧 Configuration

Key constants in `main.cpp`:
```cpp
const int MAX_FLORA = 1000;
const int MAX_HERBIVORE = 100;
const int MAX_PREDATOR = 15;

const float PREDATOR_VISION_RANGE = 150.f;
const float HERBIVORE_FOOD_VISION = 200.f;
const float HERBIVORE_PREDATOR_VISION = 120.f;
const float HERBIVORE_MATE_VISION = 150.f;
```

---

## 📁 Build Artifacts

| Directory | Purpose |
|---------|---------|
| `build/Debug/` | Debug build with symbols |
| `build/Release/` | Optimized executable for distribution |
| `build/Debug/*.dll` | SFML debug DLLs |
| `build/Release/*.dll` | SFML release DLLs |

**Required DLLs** (copy to executable directory):
- `sfml-system-3.dll` / `sfml-system-d-3.dll`
- `sfml-window-3.dll` / `sfml-window-d-3.dll`
- `sfml-graphics-3.dll` / `sfml-graphics-d-3.dll`

---

## 📚 Documentation

| File | Description |
|------|-------------|
| `history/07-step7.md` | Detailed ecosystem balancing history (Steps 7.0-7.8) |
| `history/08-step8.md` | Native SFML UI & Input Handling implementation |
| `history/02-step2.md` | Early development steps |

---

## 🐛 Known Limitations

| Limitation | Status |
|------------|--------|
| No slider interaction (visual only) | Known |
| No text input for population values | Known |
| Static button coordinates (no window resize) | Known |
| No drag-and-drop | Known |
| No tooltip system | Known |
| Sexual reproduction unused | Legacy code |

---

## 🛣 Roadmap

- [ ] Sexual reproduction integration
- [ ] Species speciation (divergent evolution)
- [ ] Environmental factors (seasons, disasters)
- [ ] Neural network brains for entities
- [ ] Multi-threaded spatial grid
- [ ] Save/Load simulation state
- [ ] WebAssembly port

---

## 📜 License

MIT License — See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- **SFML Team** — Excellent multimedia library
- **C++ Community** — Modern C++ best practices
- **Biological Inspiration** — Lotka-Volterra dynamics, optimal foraging theory

---

*Built with ❤️ using C++17 and SFML 3.0*