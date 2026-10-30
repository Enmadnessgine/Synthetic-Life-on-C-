#include "Entity.h"

Entity::Entity(sf::Vector2f position, sf::Vector2f velocity, OrganismType type)
    : position_(position), velocity_(velocity), energy_(100.f), type_(type)
{
    shape_.setPosition(position_);

    switch (type) {
        case OrganismType::Flora:
            shape_.setRadius(20.f);
            shape_.setFillColor(sf::Color::Green);
            genome_.maxSpeed = 0.f;
            break;
        case OrganismType::Herbivore:
            shape_.setRadius(15.f);
            shape_.setFillColor(sf::Color::Yellow);
            genome_.maxSpeed = 150.f;
            break;
        case OrganismType::Predator:
            shape_.setRadius(25.f);
            shape_.setFillColor(sf::Color::Red);
            genome_.maxSpeed = 200.f;
            break;
    }
}

void Entity::update(float dt, const sf::Vector2u& windowSize) {
    // Metabolism: energy decreases based on maxSpeed
    float metabolism_cost = genome_.maxSpeed * 0.02f;
    energy_ -= metabolism_cost * dt;

    // Bounce off walls
    if (position_.x <= 0.f || position_.x >= static_cast<float>(windowSize.x)) {
        velocity_.x *= -1.f;
    }
    if (position_.y <= 0.f || position_.y >= static_cast<float>(windowSize.y)) {
        velocity_.y *= -1.f;
    }

    // Visual feedback: alpha and color based on energy levels
    float alpha = energy_ / 100.f;
    if (energy_ < 30.f) {
        // Low energy: dark red
        unsigned char r = static_cast<unsigned char>(255 * (energy_ / 30.f));
        unsigned char g = static_cast<unsigned char>(255 * (energy_ / 30.f));
        unsigned char b = 0;
        shape_.setFillColor(sf::Color(r, g, b, static_cast<unsigned char>(255 * alpha)));
    } else if (energy_ < 60.f) {
        // Medium energy: yellowish
        unsigned char r = 255;
        unsigned char g = static_cast<unsigned char>(255 * (energy_ / 100.f));
        unsigned char b = 0;
        shape_.setFillColor(sf::Color(r, g, b, static_cast<unsigned char>(255 * alpha)));
    } else {
        // High energy: green
        shape_.setFillColor(sf::Color(0, 255, 0, static_cast<unsigned char>(255 * alpha)));
    }

    shape_.setPosition(position_);
}

void Entity::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
}