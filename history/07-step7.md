# Step 7: Final Ecosystem Simulation (FSM v6 - Flora Visibility Fixed)

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

## Visual Hierarchy (Step 7.3)
- **Flora:** radius 9, semi-transparent green (alpha 210) — фоновий шар
- **Herbivore:** radius 12, bright gold (255,215,0,220) with orange outline — середній шар
- **Predator:** radius 18, opaque bright red (255,0,0,255) with yellow outline — верхній шар
- **Initial speeds increased:** Herbivore ±150 (was ±100), Predator ±200 (was ±100)
- **Draw order layered:** Flora drawn first → Herbivores → Predators last (predators always visible on top)
- **Energy feedback:** alpha modulated by energy, base type colour preserved (no longer overridden by energy-based color shifts)
- **Black background** for maximum contrast
- **CRITICAL FIX:** `position_ += velocity_ * dt` added to `Entity::update()` — entities now actually move (was missing, causing all organisms to stand still)

## Finite State Machine (FSM) & Re-balancing (Step 7.4)

### FSM: стани поведінки
- **Herbivore (4 стани):** 
  - `SEEK_FOOD` — енергія < 60%, шукає найближчу флору
  - `FLEE` — хижак у радіусі 180px, втеча з +20% швидкості (264), найвищий пріоритет
  - `SEEK_MATE` — енергія > 80%, рух до центру маси сусідніх травоїдних
  - `WANDER` — стандартний стан, випадковий напрямок кожні 2.5с
- **Predator (2 стани):**
  - `SEEK_PREY` — травоїдна у радіусі 250px, переслідування
  - `WANDER` — повільне блукання (економія енергії), зміна напрямку кожні 4с

### Обмежене поле зору (реалістичне)
- **Predator:** 250px (не бачать весь екран!)
- **Herbivore:** їжа 200px, хижаки 180px, партнери 150px

### Re-balancing (портяння травоїдних від вимирання)
- **Метаболізм:** Herbivore 0.015→0.010 (економніші), Predator 0.025→0.022 (швидше голодують)
- **Енергія від поїдання:** Herbivore 80→120 (флора більш поживна), Predator 80→100 (м'ясо менш поживне)
- **Початкові умови:** Herbivore 20→40 шт., енергія 150→250; Predator 8 шт., енергія 300
- **Швидкості:** Herbivore max 220, FLEE 264 (+20% адреналін); Predator 200 (ніжче за FLEE травоїдних)
- **Розмноження:** Herbivore поріг 80 (частіше), Predator поріг 400 (рідше)

### Технічні покращення
- **Плавні переходи (lerp):** `current + (target - current) * 0.15` — без "смикання"
- **Пріоритети FSM:** FLEE > SEEK_MATE > SEEK_FOOD > WANDER (травоїдні); SEEK_PREY > WANDER (хижаки)
- **SpatialGrid:** ефективний пошук сусідів за обмеженими радіусами (не O(N²))
- **Const correctness:** збережено архітектуру, використано існуючі методи

## Critical Bug Fixes (Step 7.5) — Fix Herbivore Extinction

### Root Causes Identified
1. **FLEE state blocked eating** — herbivores in FLEE could not eat, starved while fleeing
2. **Reproduction thresholds too high** — `std::max(80, 220*1.0) = 220` for herbivores, `max(400, 200*3) = 600` for predators
3. **Predator vision too wide** — 250px (nearly full screen), herbivores perpetually fleeing
3. **FLEE never ended** — no timer, herbivores fled indefinitely
4. **Flora invisible at MAX_FLORA** — radius 5, alpha 150, invisible against black background

### Fixes Applied
- **Reproduction thresholds fixed:** Herbivore `max(80, 220*0.5) = 110`; Predator `max(400, 200*1.5) = 400`
- **Vision ranges reduced:** Predator 250→150px; Herbivore predator detection 180→120px
- **FLEE timer added:** 2.5s auto-exit back to energy-based state (SEEK_FOOD/SEEK_MATE/WANDER)
- **Eating allowed during FLEE** — herbivores can eat while fleeing
- **Flora visibility:** radius 5→7, alpha 150→180
- **Predator vision reduced:** 250→150px (realistic, not full-screen)
- **Herbivore predator detection reduced:** 180→120px (less panic)

## Flora Visibility Fixes (Step 7.6)

### Root Causes Identified
1. **Alpha divisor wrong for flora** — used 250 (herbivore max energy) instead of 100 (flora max energy), making flora alpha 0.4 at full energy
2. **Flora died in one bite** — eating damage 100 vs flora starting energy 100 = instant death
3. **Flora too small and faint** — radius 7, alpha 180, hard to see against black background
4. **Low minimum alpha floor (0.1)** — flora became nearly invisible at low energy

### Fixes Applied
- **Type-specific alpha divisors:** Flora 100, Herbivore 250, Predator 300 — correct relative brightness
- **Minimum alpha floor raised:** 0.1 → 0.3 for flora (always visible)
- **Flora starting energy:** 100 → 200 (survives one bite)
- **Eating damage reduced:** 100 → 50 (flora survives one bite, herbivore still gains 120)
- **Flora visuals enhanced:** radius 7→9, base alpha 180→210
- **Type-specific alpha divisors in Entity::update():** Flora/100, Herbivore/250, Predator/300

## Біологічний сенс (для пояснення викладачу)
- **Енергетична піраміда:** на кожному трофічному рівні втрачається ~90% енергії (правило 10%).
  Тому популяція стабілізується у пропорції приблизно 100:10:1 (Флора:Травоїдні:Хижаки).
- **Голодова смерть:** істоми, енергія яких вичерпана до 0, видаляються — природний механізм регулювання.
- **Вищий трофічний рівень = вищий поріг розмноження та вища метаболізм-вартість.**
- **Візуальна ієрархія:** хижаки (верхній трофічний рівень) малюються поверх травоїдних, травоїдні поверх флори.
- **FSM поведінка:** істоти приймають рішення на основі внутрішнього стану та оточення — не випадковий рух.
- **FLEE таймер:** імітує адреналіновий ризик — короткостковий вирив втечі, потім повернення до життєво важливих задач.

## Toroidal World (Step 7.7) — Wrap-around Screen
- **Toroidal topology:** світ має топологію тора — клітини, що виходять за лівий край, з'являються з правого, і навпаки (теж саме для верху/низу)
- **Виправлено проблему:** хижаки більше не легко ловлять травоїдних, які "заглючують" у кутах екрану — тепер травоїдні можуть втікати через край екрану
- **SpatialGrid:** коректно оновлюється кожен кадр після wrap-around, бо grid перебудовується кожен кадр

## Key Files Modified
| File | Change |
|------|--------|
| `src/main.cpp` | Full FSM interaction loop, limited vision, re-balancing, lerp smoothing, layered draw, fixed thresholds, flora energy/damage constants |
| `src/entities/Entity.h` | FSM enums, state fields, `fleeTimer`, getters/setters with logging |
| `src/entities/Entity.cpp` | FSM state transitions, flee timer auto-exit, type-specific metabolism, type-specific alpha divisors, wander behavior, visual feedback, flora visibility |
| `src/spatial/SpatialGrid.h` | Added `cellSize` ctor param, `clear()`, `getNearbyEntitiesOfType()` |
| `src/spatial/SpatialGrid.cpp` | Implementations for new ctor, `clear()`, `getNearbyEntitiesOfType()` |

## Build
- CMake generator: Visual Studio 17 2022 (x64) — SFML 3.0 libs are MSVC-format, incompatible with MinGW
- Executable: `build/Debug/ArtificialLife.exe`
- Requires SFML DLLs next to `.exe`: `sfml-system-3.dll`, `sfml-window-3.dll`, `sfml-graphics-3.dll`

## Unused Code
- `src/reproduction/IReproductionStrategy.h`, `AsexualStrategy.h`, `SexualStrategy.h` exist but are not included in `CMakeLists.txt` or `main.cpp`; reproduction logic is inlined in `main.cpp`
- `SexualStrategy.h` redefines `IReproductionStrategy` and `AsexualStrategy` (duplicate class definitions)