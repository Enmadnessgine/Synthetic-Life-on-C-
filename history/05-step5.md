# Step 5: Genome/DNA Traits Inheritance

## Changes Committed
- Added `Genome` struct with inheritable genetic traits
- Extended `Entity` class with `genome` member
- Added inheritance and mutation logic during reproduction
- Enhanced `AsexualStrategy` and `SexualStrategy` to use genome-based trait passing
- Population now inherits genetic characteristics across generations

## Genome Struct Design
- **Traits Controlled:**
  - `float maxSpeed` — maximum movement speed magnitude
  - `float size` — circle radius scale factor
  - `sf::Color color` — entity color
  - `float visionRange` — how far the entity can "see" for interactions
  - `float mutationRate` — probability of trait mutation during reproduction (0.0 to 1.0)
- **Default Values:** `maxSpeed = 200.f`, `size = 20.f`, `color = sf::Color::Green`, `visionRange = 150.f`, `mutationRate = 0.1f`

## Genome Inheritance Rules
- **Asexual Reproduction:** Offspring inherits parent's genome, then traits mutate based on `mutationRate`
  - Each trait has independent mutation probability
  - Mutated value = parent value + random offset in range `[-range, range]`
  - Range defaults: `maxSpeed ± 20.f`, `size ± 5.f`, `visionRange ± 30.f`
- **Sexual Reproduction:** Offspring inherits blended traits from both parents
  - `maxSpeed = (parent1.maxSpeed + parent2.maxSpeed) / 2` ± mutation
  - `size = (parent1.size + parent2.size) / 2` ± mutation
  - `color = average of parent1.color and parent2.color` ± mutation
  - `visionRange = (parent1.visionRange + parent2.visionRange) / 2` ± mutation

## Entity Class Extension
- Added `Genome genome_` member variable
- Constructor now accepts optional `Genome` parameter (defaults to defaults)
- Getters: `getGenome()` returns const reference
- Setters: `setGenome(Genome)` allows manual genome assignment

## Main Loop Integration
- Initialize entities with default genomes
- During reproduction, pass parent genomes to strategies
- Strategies mutate genomes and create offspring with new genomes
- Entities can now have unique genetic profiles that evolve across generations

## What Changed from Step 4
| Aspect | Step 4 | Step 5 |
|--------|--------|--------|
| **Reproduction** | Offspring have same traits as parents (with minor mutation) | Offspring have inheritable Genome with multiple traits |
| **Trait Control** | Simple velocity/position/radius mutation | Structured Genome: speed, size, color, vision, mutation rate |
| **Inheritance** | Simple clone or average | Blended + mutated across generations |
| **Population Evolution** | Population grows, entities look similar | Population evolves — diverse speeds, sizes, colors over time |

## Files Modified/Created
- `src/entities\Entity.h` (modified — added `Genome genome_` member, getters/setters)
- `src/reproduction/AsexualStrategy.h` (modified — uses genome-based mutation)
- `src/reproduction/SexualStrategy.h` (modified — uses genome-based blended inheritance)
- `src/main.cpp` (modified — genome initialization and passing to strategies)
- `history/05-step5.md` (new — this documentation)