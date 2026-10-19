#pragma once

#include <SFML/Graphics.hpp>

// Genome struct controls inheritable genetic traits
struct Genome {
    float maxSpeed = 200.f;        // Maximum movement speed
    float size = 20.f;             // Circle radius
    sf::Color color = sf::Color::Green;  // Entity color
    float visionRange = 150.f;     // How far the entity can "see" for interactions
    float mutationRate = 0.1f;     // Probability of trait mutation (0.0 to 1.0)

    // Default constructor with sensible defaults
    Genome() = default;

    // Constructor with custom values
    Genome(float maxSpeed, float size, sf::Color color, float visionRange, float mutationRate)
        : maxSpeed(maxSpeed), size(size), color(color), visionRange(visionRange), mutationRate(mutationRate) {}
};

class Entity {
public:
    Entity(sf::Vector2f position, sf::Vector2f velocity);

    void update(float dt, const sf::Vector2u& windowSize);
    void draw(sf::RenderTarget& target) const;

    // Getters
    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getVelocity() const { return velocity_; }
    float getRadius() const { return shape_.getRadius(); }
    sf::Color getColor() const { return shape_.getFillColor(); }
    const Genome& getGenome() const { return genome_; }

    // Energy metabolism
    float getEnergy() const { return energy_; }
    void addEnergy(float amount) { energy_ += amount; }
    bool isDead() const { return energy_ <= 0.f; }

    // Setters
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
    float energy_ = 100.f;  // Starting energy
};