#include "Entity.h"

Entity::Entity(sf::Vector2f position, sf::Vector2f velocity)
    : position_(position), velocity_(velocity)
{
    shape_.setRadius(20.f);
    shape_.setFillColor(sf::Color::Green);
    shape_.setPosition(position_);
}

void Entity::update(float dt, const sf::Vector2u& windowSize) {
    position_ += velocity_ * dt;

    // Bounce off walls
    if (position_.x <= 0.f || position_.x >= static_cast<float>(windowSize.x)) {
        velocity_.x *= -1.f;
    }
    if (position_.y <= 0.f || position_.y >= static_cast<float>(windowSize.y)) {
        velocity_.y *= -1.f;
    }

    shape_.setPosition(position_);
}

void Entity::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
}