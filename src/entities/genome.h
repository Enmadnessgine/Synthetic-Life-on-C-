#pragma once

#include <SFML/Graphics.hpp>

struct Genome {
    // Genetic traits
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