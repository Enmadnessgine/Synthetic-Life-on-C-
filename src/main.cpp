#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

// Genome struct controls inheritable genetic traits
struct Genome {
    float maxSpeed = 200.f;
    float size = 20.f;
    sf::Color color = sf::Color::Green;
    float visionRange = 150.f;
    float mutationRate = 0.1f;

    Genome() = default;
    Genome(float maxSpeed, float size, sf::Color color, float visionRange, float mutationRate)
        : maxSpeed(maxSpeed), size(size), color(color), visionRange(visionRange), mutationRate(mutationRate) {}
};

class Entity {
public:
    Entity(sf::Vector2f position, sf::Vector2f velocity);

    void update(float dt, const sf::Vector2u& windowSize);
    void draw(sf::RenderTarget& target) const;

    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getVelocity() const { return velocity_; }
    float getRadius() const { return shape_.getRadius(); }
    sf::Color getColor() const { return shape_.getFillColor(); }
    const Genome& getGenome() const { return genome_; }

    // Energy metabolism
    float getEnergy() const { return energy_; }
    void addEnergy(float amount) { energy_ += amount; }
    bool isDead() const { return energy_ <= 0.f; }

    void setVelocity(sf::Vector2f velocity) { velocity_ = velocity; }
    void setPosition(sf::Vector2f position) { position_ = position; shape_.setPosition(position); }
    void setRadius(float radius) { shape_.setRadius(radius); }
    void setColor(sf::Color color) { shape_.setFillColor(color); }
    void setGenome(const Genome& genome) { genome_ = genome; }

private:
    sf::Vector2f position_;
    sf::Vector2f velocity_;
    sf::CircleShape shape_;
    Genome genome_;
    float energy_ = 100.f;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - Step 6");
    window.setFramerateLimit(60);

    // Create initial entities with genomes and energy
    std::vector<std::unique_ptr<Entity>> entities;
    entities.push_back(std::make_unique<Entity>(sf::Vector2f(100.f, 100.f), sf::Vector2f(150.f, 100.f)));
    entities.push_back(std::make_unique<Entity>(sf::Vector2f(500.f, 300.f), sf::Vector2f(-100.f, 50.f)));
    entities.push_back(std::make_unique<Entity>(sf::Vector2f(300.f, 600.f), sf::Vector2f(200.f, -75.f)));
    entities.push_back(std::make_unique<Entity>(sf::Vector2f(800.f, 200.f), sf::Vector2f(-150.f, -100.f)));

    // Reproduction strategies
    // Note: reproduction is now energy-based, not just timer-based

    sf::Clock clock;

    while (window.isOpen()) {
        // Event handling
        if (const std::optional event = window.pollEvent(); event.has_value() && event->is<sf::Event::Closed>()) {
            window.close();
        }

        float dt = clock.restart().asSeconds();

        // 1. Update all entities (metabolism, movement, wall bounce)
        for (auto& entity : entities) {
            entity->update(dt, window.getSize());
        }

        // 2. Reproduction logic (energy-based, not just timer-based)
        // Check each entity if it can reproduce
        static float asexualTimer = 0.f;
        static float sexualTimer = 0.f;
        const float reproduceInterval = 5.f; // base interval check frequency

        asexualTimer += dt;
        sexualTimer += dt;

        // Asexual reproduction: check each entity
        if (asexualTimer >= reproduceInterval) {
            // Iterate over a copy of indices to safely handle removals
            for (int i = (int)entities.size() - 1; i >= 0; --i) {
                Entity& entity = *entities[i];
                // Can reproduce if energy >= maxSpeed * 2.0f
                if (entity.getEnergy() >= entity.getGenome().maxSpeed * 2.0f) {
                    // Clone parent with mutation
                    Genome childGenome = entity.getGenome();

                    // Mutate genome traits
                    if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                        // Mutate maxSpeed-related velocity
                        float speedMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 40.f;
                        childGenome.maxSpeed += speedMutate;
                        if (childGenome.maxSpeed < 50.f) childGenome.maxSpeed = 50.f;
                        if (childGenome.maxSpeed > 500.f) childGenome.maxSpeed = 500.f;

                        // Mutate size
                        float sizeMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.f;
                        childGenome.size += sizeMutate;
                        if (childGenome.size < 5.f) childGenome.size = 5.f;
                        if (childGenome.size > 50.f) childGenome.size = 50.f;

                        // Mutate color (change one channel)
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            int channel = rand() % 3;
                            int change = rand() % 50 - 25;
                            sf::Color c = childGenome.color;
                            if (channel == 0) c.r = std::max(0, std::min(255, c.r + change));
                            else if (channel == 1) c.g = std::max(0, std::min(255, c.g + change));
                            else c.b = std::max(0, std::min(255, c.b + change));
                            childGenome.color = c;
                        }

                        // Mutate visionRange
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            float visionMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.f;
                            childGenome.visionRange += visionMutate;
                            if (childGenome.visionRange < 30.f) childGenome.visionRange = 30.f;
                            if (childGenome.visionRange > 300.f) childGenome.visionRange = 300.f;
                        }

                        // Apply mutated genome to child entity
                        Entity child(entity.getPosition(), entity.getVelocity());
                        child.setGenome(childGenome);
                        float speedFactor = childGenome.maxSpeed / 200.f;
                        child.setVelocity(sf::Vector2f(
                            entity.getVelocity().x * speedFactor,
                            entity.getVelocity().y * speedFactor
                        ));
                        child.setRadius(childGenome.size);
                        child.setColor(childGenome.color);

                        // Parent loses 50% of current energy
                        entity.addEnergy(-entity.getEnergy() * 0.5f);

                        // Child created with the energy that was subtracted
                        // (child's energy starts at 100.f, but its properties reflect the parent's state)

                        entities.push_back(std::make_unique<Entity>(std::move(child)));
                    }
                    asexualTimer = 0.f;
                    // Only one asexual reproduction per entity per interval
                    break; 
                }
            }
        }

        // Sexual reproduction: check pairs of entities
        if (sexualTimer >= reproduceInterval) {
            if (entities.size() >= 2) {
                // Find two random distinct entities that have enough energy
                int parent1Idx = rand() % entities.size();
                int parent2Idx = rand() % entities.size();
                while (parent2Idx == parent1Idx) {
                    parent2Idx = rand() % entities.size();
                }

                Entity& parent1 = *entities[parent1Idx];
                Entity& parent2 = *entities[parent2Idx];

                // Both parents need enough energy to reproduce
                if (parent1.getEnergy() >= parent1.getGenome().maxSpeed * 2.0f &&
                    parent2.getEnergy() >= parent2.getGenome().maxSpeed * 2.0f) {

                    // Create offspring with blended genome
                    Genome childGenome;

                    // Blend traits from both parents
                    childGenome.maxSpeed = (parent1.getGenome().maxSpeed + parent2.getGenome().maxSpeed) / 2.f;
                    childGenome.size = (parent1.getGenome().size + parent2.getGenome().size) / 2.f;
                    sf::Color c1 = parent1.getGenome().color;
                    sf::Color c2 = parent2.getGenome().color;
                    sf::Color avgColor = sf::Color(
                        (c1.r + c2.r) / 2,
                        (c1.g + c2.g) / 2,
                        (c1.b + c2.b) / 2
                    );
                    childGenome.color = avgColor;
                    childGenome.visionRange = (parent1.getGenome().visionRange + parent2.getGenome().visionRange) / 2.f;
                    childGenome.mutationRate = (parent1.getGenome().mutationRate + parent2.getGenome().mutationRate) / 2.f;

                    // Apply mutation
                    if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            float speedMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 40.f;
                            childGenome.maxSpeed += speedMutate;
                            if (childGenome.maxSpeed < 50.f) childGenome.maxSpeed = 50.f;
                            if (childGenome.maxSpeed > 500.f) childGenome.maxSpeed = 500.f;
                        }
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            float sizeMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.f;
                            childGenome.size += sizeMutate;
                            if (childGenome.size < 5.f) childGenome.size = 5.f;
                            if (childGenome.size > 50.f) childGenome.size = 50.f;
                        }
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            int channel = rand() % 3;
                            int change = rand() % 50 - 25;
                            sf::Color c = childGenome.color;
                            if (channel == 0) c.r = std::max(0, std::min(255, c.r + change));
                            else if (channel == 1) c.g = std::max(0, std::min(255, c.g + change));
                            else c.b = std::max(0, std::min(255, c.b + change));
                            childGenome.color = c;
                        }
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            float visionMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.f;
                            childGenome.visionRange += visionMutate;
                            if (childGenome.visionRange < 30.f) childGenome.visionRange = 30.f;
                            if (childGenome.visionRange > 300.f) childGenome.visionRange = 300.f;
                        }
                        if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                            float rateMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
                            childGenome.mutationRate += rateMutate;
                            if (childGenome.mutationRate < 0.0f) childGenome.mutationRate = 0.0f;
                            if (childGenome.mutationRate > 1.0f) childGenome.mutationRate = 1.0f;
                        }
                    }

                    // Apply blended genome to child entity
                    Entity child((parent1.getPosition() + parent2.getPosition()) / 2.f,
                                  (parent1.getVelocity() + parent2.getVelocity()) / 2.f);
                    child.setGenome(childGenome);
                    float speedFactor = childGenome.maxSpeed / 200.f;
                    child.setVelocity(sf::Vector2f(
                        (parent1.getVelocity().x + parent2.getVelocity().x) / 2.f * speedFactor,
                        (parent1.getVelocity().y + parent2.getVelocity().y) / 2.f * speedFactor
                    ));
                    child.setRadius(childGenome.size);
                    child.setColor(childGenome.color);

                    // Both parents lose 50% of current energy
                    parent1.addEnergy(-parent1.getEnergy() * 0.5f);
                    parent2.addEnergy(-parent2.getEnergy() * 0.5f);

                    entities.push_back(std::make_unique<Entity>(std::move(child)));
                    sexualTimer = 0.f;
                }
            }
        }

// 3. "Mark and Sweep" pattern: safely remove dead entities
        // NEVER erase during iteration! Use erase-remove idiom (C++17)
        entities.erase(
            std::remove_if(entities.begin(), entities.end(), 
                [](const std::unique_ptr<Entity>& e) {
                    return e->isDead();
                }),
            entities.end()
        );

        // 4. Event handling
        if (const std::optional event = window.pollEvent(); event.has_value() && event->is<sf::Event::Closed>()) {
            window.close();
        }

        // 5. Update entities again after cleanup (for consistent state)
        // Actually, entities were already updated above, so skip re-update

        // 6. Draw
        window.clear(sf::Color::Black);
        for (const auto& entity : entities) {
            entity->draw(window);
        }
        window.display();
    }

    return 0;
}