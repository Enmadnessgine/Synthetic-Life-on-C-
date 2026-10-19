#include "Entity.h"

Entity::Entity(sf::Vector2f position, sf::Vector2f velocity)
    : position_(position), velocity_(velocity), energy_(100.f)
{
    shape_.setRadius(20.f);
    shape_.setFillColor(sf::Color::Green);
    shape_.setPosition(position_);
}

void Entity::update(float dt, const sf::Vector2u& windowSize) {
    // Metabolism: energy decreases based on maxSpeed
    float metabolism_cost = genome_.maxSpeed * 0.1f;
    energy_ -= metabolism_cost * dt;

    // Bounce off walls
    if (position_.x <= 0.f || position_.x >= static_cast<float>(windowSize.x)) {
        velocity_.x *= -1.f;
    }
    if (position_.y <= 0.f || position_.y >= static_cast<float>(windowSize.y)) {
        velocity_.y *= -1.f;
    }

    // Visual feedback: alpha based on energy (0.f = transparent, 1.f = opaque)
    float alpha = energy_ / 100.f;
    if (alpha < 0.3f) {
        // Low energy: turn red
        shape_.setFillColor(sf::Color(255, (unsigned char)(255 * alpha), 0, (unsigned char)(255 * alpha)));
    } else {
        shape_.setFillColor(sf::Color(0, 255, 0, (unsigned char)(255 * alpha)));
    }

    shape_.setPosition(position_);
}

void Entity::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
}