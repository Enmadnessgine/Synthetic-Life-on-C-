#pragma once

class Entity;

class AsexualStrategy : public IReproductionStrategy {
public:
    std::unique_ptr<Entity> reproduce(const Entity& parent, float mutationRate) const;
    std::unique_ptr<Entity> reproduce(const Entity& parent1, const Entity& parent2, float mutationRate) const;
    bool shouldReproduce(const Entity& entity, float dt) const;
};

// Definition of AsexualStrategy::reproduce (asexual - clone parent with mutation)
std::unique_ptr<Entity> AsexualStrategy::reproduce(const Entity& parent, float mutationRate) {
    auto child = std::make_unique<Entity>(parent.getPosition(), parent.getVelocity());

    // Inherit genome from parent
    Genome& parentGenome = parent.getGenome();

    // Apply mutation to genome traits if random check passes
    if (mutationRate > 0.0f && (rand() / (double)RAND_MAX) < mutationRate) {
        // Mutate maxSpeed
        if ((rand() / (double)RAND_MAX) < parentGenome.mutationRate) {
            float speedMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 40.f;
            parentGenome.maxSpeed += speedMutate;
            if (parentGenome.maxSpeed < 50.f) parentGenome.maxSpeed = 50.f;
            if (parentGenome.maxSpeed > 500.f) parentGenome.maxSpeed = 500.f;
        }

        // Mutate size
        if ((rand() / (double)RAND_MAX) < parentGenome.mutationRate) {
            float sizeMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.f;
            parentGenome.size += sizeMutate;
            if (parentGenome.size < 5.f) parentGenome.size = 5.f;
            if (parentGenome.size > 50.f) parentGenome.size = 50.f;
        }

        // Mutate color (change one color channel)
        if ((rand() / (double)RAND_MAX) < parentGenome.mutationRate) {
            int channel = rand() % 3; // 0=R, 1=G, 2=B
            int change = rand() % 50 - 25; // -25 to +25
            sf::Color c = parentGenome.color;
            if (channel == 0) c.r = std::max(0, std::min(255, c.r + change));
            else if (channel == 1) c.g = std::max(0, std::min(255, c.g + change));
            else c.b = std::max(0, std::min(255, c.b + change));
            parentGenome.color = c;
        }

        // Mutate visionRange
        if ((rand() / (double)RAND_MAX) < parentGenome.mutationRate) {
            float visionMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.f;
            parentGenome.visionRange += visionMutate;
            if (parentGenome.visionRange < 30.f) parentGenome.visionRange = 30.f;
            if (parentGenome.visionRange > 300.f) parentGenome.visionRange = 300.f;
        }

        // Mutate mutationRate itself
        if ((rand() / (double)RAND_MAX) < parentGenome.mutationRate) {
            float rateMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
            parentGenome.mutationRate += rateMutate;
            if (parentGenome.mutationRate < 0.0f) parentGenome.mutationRate = 0.0f;
            if (parentGenome.mutationRate > 1.0f) parentGenome.mutationRate = 1.0f;
        }

        // Apply mutated genome to child
        child->setGenome(parentGenome);

        // Apply derived mutations to child's immediate properties
        float speedFactor = parentGenome.maxSpeed / 200.f;
        child->setVelocity(sf::Vector2f(
            parent.getVelocity().x * speedFactor,
            parent.getVelocity().y * speedFactor
        ));

        child->setRadius(parentGenome.size);
        child->setColor(parentGenome.color);
    } else {
        // No mutation - child inherits exact properties from parent
        child->setRadius(parentGenome.size);
        child->setColor(parentGenome.color);
    }

    return child;
}

// Asexual reproduce: just returns a copy of parent1 for sexual strategy
std::unique_ptr<Entity> AsexualStrategy::reproduce(const Entity& parent1, const Entity& parent2, float mutationRate) {
    return std::make_unique<Entity>(parent1.getPosition(), parent1.getVelocity());
}

bool AsexualStrategy::shouldReproduce(const Entity& entity, float dt) {
    static float timer = 0.0f;
    timer += dt;
    // Only reproduce if energy >= maxSpeed * 2.0f (energy reserve threshold)
    // And timer condition met
    return timer >= 5.0f && entity.getEnergy() >= entity.getGenome().maxSpeed * 2.0f;
}