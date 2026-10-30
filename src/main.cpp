#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <SFML/Graphics.hpp>

#include "entities/Entity.h"
#include "spatial/SpatialGrid.h"

// Допоміжна функція для розрахунку відстані
float getDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Ініціалізація генератора випадкових чисел

    // 1. Налаштування вікна
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - Final Ecosystem");
    window.setFramerateLimit(60);
    sf::Clock clock;

    // 2. Ініціалізація SpatialGrid (світ 1024x768, комірка 100px)
    SpatialGrid grid(1024.f, 768.f, 100.f);

    // 3. Створення початкової популяції
    std::vector<std::unique_ptr<Entity>> entities;
    
    std::cout << "=== Ініціалізація екосистеми ===" << std::endl;

    // Флора (150 шт.)
    for (int i = 0; i < 150; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        auto flora = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(0.f, 0.f), Entity::OrganismType::Flora);
        flora->addEnergy(100.f);
        entities.push_back(std::move(flora));
    }

    // Травоїдні (30 шт.)
    for (int i = 0; i < 30; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto herb = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Herbivore);
        herb->addEnergy(150.f);
        entities.push_back(std::move(herb));
    }

    // Хижаки (10 шт.)
    for (int i = 0; i < 10; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto pred = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Predator);
        pred->addEnergy(200.f);
        entities.push_back(std::move(pred));
    }
    std::cout << "Створено істот. Загальна кількість: " << entities.size() << "\n" << std::endl;

    // Таймери для розмноження
    float asexualTimer = 0.f;

    // 4. Головний цикл симуляції
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Запобігання стрибкам при лагах

        // --- Обробка подій (SFML 3.0 API) ---
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // --- КРОК А: Оновлення фізики та метаболізму ---
        for (auto& entity : entities) {
            entity->update(dt, window.getSize());
        }

        // --- КРОК Б: Оновлення SpatialGrid ---
        grid.clear();
        for (size_t i = 0; i < entities.size(); ++i) {
            grid.addEntity(entities[i]->getPosition(), i);
        }

        // --- КРОК В: Харчовий ланцюг (Взаємодії) ---
        for (size_t i = 0; i < entities.size(); ++i) {
            auto& entity = entities[i];
            if (entity->isDead()) continue;

            Entity::OrganismType type = entity->getType();

            if (type == Entity::OrganismType::Flora) {
                // Фотосинтез
                entity->addEnergy(25.f * dt); 
            } 
            else if (type == Entity::OrganismType::Herbivore) {
                auto targets = grid.getNearbyEntitiesOfType(entity->getPosition(), entity->getGenome().visionRange, Entity::OrganismType::Flora, entities);
                if (!targets.empty()) {
                    int closestIdx = targets[0];
                    float minDist = getDistance(entity->getPosition(), entities[closestIdx]->getPosition());
                    
                    for (int idx : targets) {
                        float dist = getDistance(entity->getPosition(), entities[idx]->getPosition());
                        if (dist < minDist) {
                            minDist = dist;
                            closestIdx = idx;
                        }
                    }

                    auto& target = entities[closestIdx];
                    if (!target->isDead()) {
                        // Рух до цілі (Seek)
                        sf::Vector2f dir = target->getPosition() - entity->getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) {
                            entity->setVelocity((dir / len) * entity->getGenome().maxSpeed);
                        }

                        // Поїдання
                        float collisionDist = entity->getGenome().size + target->getGenome().size;
                        if (minDist < collisionDist) {
                            entity->addEnergy(60.f);       
                            target->addEnergy(-100.f);     
                        }
                    }
                }
            } 
            else if (type == Entity::OrganismType::Predator) {
                auto targets = grid.getNearbyEntitiesOfType(entity->getPosition(), entity->getGenome().visionRange, Entity::OrganismType::Herbivore, entities);
                if (!targets.empty()) {
                    int closestIdx = targets[0];
                    float minDist = getDistance(entity->getPosition(), entities[closestIdx]->getPosition());
                    
                    for (int idx : targets) {
                        float dist = getDistance(entity->getPosition(), entities[idx]->getPosition());
                        if (dist < minDist) {
                            minDist = dist;
                            closestIdx = idx;
                        }
                    }

                    auto& target = entities[closestIdx];
                    if (!target->isDead()) {
                        sf::Vector2f dir = target->getPosition() - entity->getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) {
                            entity->setVelocity((dir / len) * entity->getGenome().maxSpeed);
                        }

                        float collisionDist = entity->getGenome().size + target->getGenome().size;
                        if (minDist < collisionDist) {
                            entity->addEnergy(100.f);      
                            target->addEnergy(-100.f);     
                        }
                    }
                }
            }
        }

        // --- КРОК Г: Розмноження ---
        asexualTimer += dt;
        
        std::vector<std::unique_ptr<Entity>> newborns;
        
        if (asexualTimer >= 4.0f) {
            for (auto& entity : entities) {
                if (entity->isDead()) continue;
                
                float threshold = std::max(120.f, entity->getGenome().maxSpeed * 1.2f);
                if (entity->getEnergy() >= threshold) {
                    float energyToPass = entity->getEnergy() * 0.4f;
                    entity->addEnergy(-energyToPass);

                    Genome childGenome = entity->getGenome();
                    childGenome.maxSpeed += static_cast<float>((std::rand() % 21) - 10); // ±10 мутація
                    childGenome.size += static_cast<float>((std::rand() % 5) - 2);       // ±2 мутація
                    
                    sf::Vector2f childPos = entity->getPosition() + sf::Vector2f(
                        static_cast<float>(std::rand() % 40 - 20), 
                        static_cast<float>(std::rand() % 40 - 20)
                    );
                    
                    sf::Vector2f childVel(0.f, 0.f);
                    if (entity->getType() != Entity::OrganismType::Flora) {
                        childVel = sf::Vector2f(
                            static_cast<float>(std::rand() % 200 - 100), 
                            static_cast<float>(std::rand() % 200 - 100)
                        );
                    }

                    auto child = std::make_unique<Entity>(childPos, childVel, entity->getType());
                    child->setGenome(childGenome);
                    child->addEnergy(energyToPass);
                    newborns.push_back(std::move(child));
                }
            }
            asexualTimer = 0.f; // Таймер скидається завжди, щоб уникнути спаму
        }

        for (auto& child : newborns) {
            entities.push_back(std::move(child));
        }

        // --- КРОК Д: Очищення мертвих істот (Mark and Sweep) ---
        entities.erase(
            std::remove_if(entities.begin(), entities.end(), 
                [](const std::unique_ptr<Entity>& e) { return e->isDead(); }),
            entities.end()
        );

        // --- КРОК Е: Рендеринг ---
        window.clear(sf::Color::Black);
        for (const auto& entity : entities) {
            entity->draw(window);
        }
        window.display();

        // Відладочний вивід статистики кожну секунду
        static int frameCount = 0;
        if (++frameCount % 60 == 0) {
            int flora = 0, herb = 0, pred = 0;
            for (const auto& e : entities) {
                if (e->getType() == Entity::OrganismType::Flora) flora++;
                else if (e->getType() == Entity::OrganismType::Herbivore) herb++;
                else if (e->getType() == Entity::OrganismType::Predator) pred++;
            }
            std::cout << "Статистика -> Flora: " << flora 
                      << " | Herbivore: " << herb 
                      << " | Predator: " << pred << std::endl;
        }
    }

    return 0;
}
