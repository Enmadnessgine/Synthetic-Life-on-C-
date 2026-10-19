#pragma once

class Entity;

class IReproductionStrategy {
public:
    virtual ~IReproductionStrategy() = default;
    virtual std::unique_ptr<Entity> reproduce(const Entity& parent, float mutationRate) const = 0;
    virtual std::unique_ptr<Entity> reproduce(const Entity& parent1, const Entity& parent2, float mutationRate) const = 0;
    virtual bool shouldReproduce(const Entity& entity, float dt) const = 0;
};

// Asexual reproduction strategy - clones parent with optional mutation
class AsexualStrategy : public IReproductionStrategy {
public:
    // Clone parent with optional mutation
    std::unique_ptr<Entity> reproduce(const Entity& parent, float mutationRate) const override {
        auto child = std::make_unique<Entity>(parent.getPosition(), parent.getVelocity());

        // Inherit genome from parent
        Genome& parentGenome = parent.getGenome();

        // Apply mutation if random check passes
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
                int channel = rand() % 3;
                int change = rand() % 50 - 25;
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

            // Apply derived mutations
            float speedFactor = parentGenome.maxSpeed / 200.f;
            child->setVelocity(sf::Vector2f(
                parent.getVelocity().x * speedFactor,
                parent.getVelocity().y * speedFactor
            ));
            child->setRadius(parentGenome.size);
            child->setColor(parentGenome.color);
        } else {
            child->setRadius(parentGenome.size);
            child->setColor(parentGenome.color);
        }

        return child;
    }

    // For sexual strategy, return a copy of parent1
    std::unique_ptr<Entity> reproduce(const Entity& parent1, const Entity& parent2, float mutationRate) const override {
        return std::make_unique<Entity>(parent1.getPosition(), parent1.getVelocity());
    }

    // Asexual reproduction trigger: every 5 seconds
    bool shouldReproduce(const Entity& entity, float dt) const override {
        static float timer = 0.0f;
        timer += dt;
        return timer >= 5.0f;
    }
};

// Sexual reproduction strategy - blends two parents with optional mutation
class SexualStrategy : public IReproductionStrategy {
public:
    // Blend two parents with optional mutation
    std::unique_ptr<Entity> reproduce(const Entity& parent1, const Entity& parent2, float mutationRate) const override {
        auto child = std::make_unique<Entity>((parent1.getPosition() + parent2.getPosition()) / 2.f,
                                              (parent1.getVelocity() + parent2.getVelocity()) / 2.f);

        // Inherit and blend genomes from both parents
        Genome& parent1Genome = parent1.getGenome();
        Genome& parent2Genome = parent2.getGenome();
        Genome childGenome;

        // Blend each trait
        childGenome.maxSpeed = (parent1Genome.maxSpeed + parent2Genome.maxSpeed) / 2.f;
        childGenome.size = (parent1Genome.size + parent2Genome.size) / 2.f;
        sf::Color c1 = parent1Genome.color;
        sf::Color c2 = parent2Genome.color;
        sf::Color avgColor = sf::Color(
            (c1.r + c2.r) / 2,
            (c1.g + c2.g) / 2,
            (c1.b + c2.b) / 2
        );
        childGenome.color = avgColor;
        childGenome.visionRange = (parent1Genome.visionRange + parent2Genome.visionRange) / 2.f;
        childGenome.mutationRate = (parent1Genome.mutationRate + parent2Genome.mutationRate) / 2.f;

        // Apply mutation if random check passes
        if (mutationRate > 0.0f && (rand() / (double)RAND_MAX) < mutationRate) {
            // Mutate maxSpeed
            if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                float speedMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 40.f;
                childGenome.maxSpeed += speedMutate;
                if (childGenome.maxSpeed < 50.f) childGenome.maxSpeed = 50.f;
                if (childGenome.maxSpeed > 500.f) childGenome.maxSpeed = 500.f;
            }

            // Mutate size
            if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                float sizeMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.f;
                childGenome.size += sizeMutate;
                if (childGenome.size < 5.f) childGenome.size = 5.f;
                if (childGenome.size > 50.f) childGenome.size = 50.f;
            }

            // Mutate color (change one color channel)
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

            // Mutate mutationRate itself
            if ((rand() / (double)RAND_MAX) < childGenome.mutationRate) {
                float rateMutate = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
                childGenome.mutationRate += rateMutate;
                if (childGenome.mutationRate < 0.0f) childGenome.mutationRate = 0.0f;
                if (childGenome.mutationRate > 1.0f) childGenome.mutationRate = 1.0f;
            }

            // Apply blended genome to child
            child->setGenome(childGenome);

            // Apply derived properties
            float speedFactor = childGenome.maxSpeed / 200.f;
            child->setVelocity(sf::Vector2f(
                (parent1.getVelocity().x + parent2.getVelocity().x) / 2.f * speedFactor,
                (parent1.getVelocity().y + parent2.getVelocity().y) / 2.f * speedFactor
            ));

            child->setRadius(childGenome.size);
            child->setColor(childGenome.color);
        } else {
            // No mutation - child inherits blended genome
            child->setGenome(childGenome);
            child->setRadius(childGenome.size);
            child->setColor(childGenome.color);
        }

        return child;
    }

    // Asexual reproduce: just returns a copy of parent1
    std::unique_ptr<Entity> reproduce(const Entity& parent, float mutationRate) const override {
        return std::make_unique<Entity>(parent.getPosition(), parent.getVelocity());
    }

    // Sexual reproduction trigger: every 7 seconds
    bool shouldReproduce(const Entity& entity, float dt) const override {
        static float timer = 0.0f;
        timer += dt;
        return timer >= 7.0f;
    }
};