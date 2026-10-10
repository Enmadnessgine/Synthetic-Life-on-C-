#pragma once

#include <SFML/Graphics.hpp>

class Entity {
public:
    Entity(sf::Vector2f position, sf::Vector2f velocity);

    void update(float dt, const sf::Vector2u& windowSize);
    void draw(sf::RenderTarget& target) const;

private:
    sf::Vector2f position_;
    sf::Vector2f velocity_;
    sf::CircleShape shape_;
};