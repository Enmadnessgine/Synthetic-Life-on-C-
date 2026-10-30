#include "Entity.h"

Entity::Entity(sf::Vector2f position, sf::Vector2f velocity, OrganismType type)
    : position_(position), velocity_(velocity), energy_(100.f), type_(type)
{
    shape_.setPosition(position_);

    switch (type) {
        case OrganismType::Flora:
            // Флора: маленькі напівпрозорі зелені кружечки (fonівий шар)
            // Радіус 5, прозорість 150 — вона на задньому плані.
            shape_.setRadius(5.f);
            shape_.setFillColor(sf::Color(0, 255, 0, 150));
            shape_.setOutlineThickness(0.f);
            genome_.maxSpeed = 0.f;
            break;
        case OrganismType::Herbivore:
            // Травоїдні: великі яскраві жовті/золоті кружечки з помаранчевим контуром
            // Радіус 12, прозорість 220 — середній шар візуальної ієрархії.
            shape_.setRadius(12.f);
            shape_.setFillColor(sf::Color(255, 215, 0, 220));
            shape_.setOutlineColor(sf::Color(255, 140, 0, 220));
            shape_.setOutlineThickness(2.f);
            genome_.maxSpeed = 150.f;
            break;
        case OrganismType::Predator:
            // Хижаки: великі яскраво-червоні кружечки з жовтим контуром (верхній шар)
            // Радіус 18, непрозорі — завжди видно поверх усіх інших істот.
            shape_.setRadius(18.f);
            shape_.setFillColor(sf::Color(255, 0, 0, 255));
            shape_.setOutlineColor(sf::Color(255, 255, 0, 255));
            shape_.setOutlineThickness(3.f);
            genome_.maxSpeed = 200.f;
            break;
    }
}

void Entity::update(float dt, const sf::Vector2u& windowSize) {
    // --- ІНТЕГРАЦІЯ ПОЗИЦІЇ (критично для руху) ---
    position_ += velocity_ * dt;

    // Metabolism: energy decreases based on maxSpeed
    // Хижаки витрачають більше енергії на метаболізм (вищий обмін речовин)
    float metabolism_multiplier = (type_ == Entity::OrganismType::Predator) ? 0.025f : 0.015f;
    float metabolism_cost = genome_.maxSpeed * metabolism_multiplier;
    energy_ -= metabolism_cost * dt;

    // Bounce off walls
    if (position_.x <= 0.f || position_.x >= static_cast<float>(windowSize.x)) {
        velocity_.x *= -1.f;
    }
    if (position_.y <= 0.f || position_.y >= static_cast<float>(windowSize.y)) {
        velocity_.y *= -1.f;
    }

    // Visual feedback: alpha based on energy, preserve base type colour
    float alpha = energy_ / 100.f;
    if (alpha < 0.1f) alpha = 0.1f; // Ніколи не стає цілком невидимою
    sf::Color baseColor = shape_.getFillColor();
    shape_.setFillColor(sf::Color(baseColor.r, baseColor.g, baseColor.b,
        static_cast<unsigned char>(baseColor.a * alpha)));

    shape_.setPosition(position_);
}

void Entity::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
}