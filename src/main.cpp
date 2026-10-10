#include <SFML/Window.hpp>
#include <vector>
#include <memory>
#include "entities/Entity.h"

int main() {
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - Step 2");
    window.setFramerateLimit(60);

    // Create entities container
    std::vector<std::unique_ptr<Entity>> entities;
    entities.push_back(std::make_unique<Entity>(sf::Vector2f(100.f, 100.f), sf::Vector2f(150.f, 100.f)));

    sf::Clock clock;

    while (window.isOpen()) {
        // Event handling (SFML 3.0 style)
        if (const std::optional event = window.pollEvent(); event.has_value() && event->is<sf::Event::Closed>()) {
            window.close();
        }

        float dt = clock.restart().asSeconds();

        // Update entities
        for (auto& entity : entities) {
            entity->update(dt, window.getSize());
        }

        // Draw
        window.clear(sf::Color::Black);
        for (const auto& entity : entities) {
            entity->draw(window);
        }
        window.display();
    }

    return 0;
}