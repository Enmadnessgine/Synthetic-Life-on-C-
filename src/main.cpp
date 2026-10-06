#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - Step 1");
    window.setFramerateLimit(60);

    sf::CircleShape circle(50.f);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({window.getSize().x / 2.f, window.getSize().y / 2.f});

    sf::Vector2f velocity(200.f, 150.f);
    sf::Clock clock;

    while (window.isOpen()) {
        if (const std::optional event = window.pollEvent(); event->is<sf::Event::Closed>())
            window.close();

        float dt = clock.restart().asSeconds();

        circle.move(velocity * dt);

        sf::Vector2u size = window.getSize();
        if (circle.getPosition().x <= 0 || circle.getRadius() * 2 >= size.x)
            velocity.x *= -1;
        if (circle.getPosition().y <= 0 || circle.getRadius() * 2 >= size.y)
            velocity.y *= -1;

        window.clear(sf::Color::Black);
        window.draw(circle);
        window.display();
    }

    return 0;
}