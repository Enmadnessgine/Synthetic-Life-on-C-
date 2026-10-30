#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

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
    enum class OrganismType { Flora, Herbivore, Predator };

    // === FSM: Stani поведінки травоїдних (Herbivore) ===
    // SEEK_FOOD   — пошук їжі (флори) при низькій енергії
    // FLEE        — втеча від хижака (найвищий пріоритет — виживання)
    // SEEK_MATE   — пошук партнера для розмноження при высокій енергії
    // WANDER      — блукання (стандартний стан, немає нагальних потреб)
    enum class HerbivoreState {
        SEEK_FOOD,
        FLEE,
        SEEK_MATE,
        WANDER
    };

    // === FSM: Stani поведінки хижаків (Predator) ===
    // SEEK_PREY   — переслідування здобичі (травоїдних)
    // WANDER      — повільне блукання для економії енергії
    enum class PredatorState {
        SEEK_PREY,
        WANDER
    };

    Entity(sf::Vector2f position, sf::Vector2f velocity, OrganismType type);

    void update(float dt, const sf::Vector2u& windowSize);
    void draw(sf::RenderTarget& target) const;

    // Getters
    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getVelocity() const { return velocity_; }
    float getRadius() const { return shape_.getRadius(); }
    sf::Color getColor() const { return shape_.getFillColor(); }
    const Genome& getGenome() const { return genome_; }
    OrganismType getType() const { return type_; }

    // Energy metabolism
    float getEnergy() const { return energy_; }
    void addEnergy(float amount) { energy_ += amount; }
    void setEnergy(float amount) { energy_ = amount; }
    bool isDead() const { return energy_ <= 0.f; }

    // === FSM: Getters/Setters для станів ===
    HerbivoreState getHerbivoreState() const { return currentHerbivoreState; }
    PredatorState getPredatorState() const { return currentPredatorState; }
    float getWanderTimer() const { return wanderTimer; }
    sf::Vector2f getWanderDirection() const { return wanderDirection; }

    // Зміна стану з логуванням для дебагу
    void setHerbivoreState(HerbivoreState state);
    void setPredatorState(PredatorState state);
    void setWanderTimer(float t) { wanderTimer = t; }
    void setWanderDirection(sf::Vector2f dir) { wanderDirection = dir; }

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
    float energy_ = 100.f;
    OrganismType type_;

    // === FSM: Поля стану ===
    HerbivoreState currentHerbivoreState = HerbivoreState::WANDER;
    PredatorState currentPredatorState = PredatorState::WANDER;
    float wanderTimer = 0.f;
    sf::Vector2f wanderDirection = sf::Vector2f(0.f, 0.f);
};
