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

// Ліміти популяції для стабілізації екосистеми (пропорція енергетичної піраміди ~100:10:1)
// Біологічний сенс: на кожному трофічному рівні втрачається ~90% енергії (правило 10%),
// тому хижаків має бути на порядок менше, ніж травоїдів, а травоїдів — ніж рослин.
const int MAX_FLORA = 1000;
const int MAX_HERBIVORE = 100;
const int MAX_PREDATOR = 15;

// Допоміжна функція для розрахунку відстані
float getDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Ініціалізація генератора випадкових чисел

    // 1. Налаштування вікна
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - Balanced Ecosystem");
    window.setFramerateLimit(60);
    sf::Clock clock;

    // 2. Ініціалізація SpatialGrid (світ 1024x768, комірка 100px)
    SpatialGrid grid(1024.f, 768.f, 100.f);

    // 3. Створення початкової популяції
    // Початкові кількості зменшено для швидшо стабілізації екосистеми.
    // Біологічний сенс: початкова біомаса має бути невеликою, щоб екосистема могла
    // органічно зрости до рівноважної довкілях.
    std::vector<std::unique_ptr<Entity>> entities;
    
    std::cout << "=== Ініціалізація збалансованої екосистеми ===" << std::endl;

    // Флора (100 шт.) — продуктенти першого трофічного рівня
    // Розмножуються шляхом фотосинтезу, не потребують їжі.
    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        auto flora = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(0.f, 0.f), Entity::OrganismType::Flora);
        flora->addEnergy(100.f);
        entities.push_back(std::move(flora));
    }

    // Травоїдні (20 шт.) — споживачі другого трофічного рівня
    // Їдять флору, отримуючи частину енергії рослин (згідно з правилом 10%).
    for (int i = 0; i < 20; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto herb = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Herbivore);
        herb->addEnergy(150.f);
        entities.push_back(std::move(herb));
    }

    // Хижаки (8 шт.) — споживачі третього трофічного рівня
    // Їдять травоїдних, отримаючи ще меншу частку енергії з ланцюга.
    for (int i = 0; i < 8; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto pred = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Predator);
        pred->addEnergy(200.f);
        entities.push_back(std::move(pred));
    }
    std::cout << "Створено істот. Загальна кількість: " << entities.size() << "\n" << std::endl;

    // Таймер для розмноження (асексуальне розмноження кожні 4 секунди)
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

        // Підрахунок поточної популяції для лімітів (енергетична піраміда)
        int floraCount = 0, herbCount = 0, predCount = 0;
        for (const auto& e : entities) {
            if (e->getType() == Entity::OrganismType::Flora) floraCount++;
            else if (e->getType() == Entity::OrganismType::Herbivore) herbCount++;
            else if (e->getType() == Entity::OrganismType::Predator) predCount++;
        }

        // --- КРОК В: Харчовий ланцюг (Взаємодії) ---
        for (size_t i = 0; i < entities.size(); ++i) {
            auto& entity = entities[i];
            if (entity->isDead()) continue;

            Entity::OrganismType type = entity->getType();

            if (type == Entity::OrganismType::Flora) {
                // Фотосинтез — флора накопичує енергію зі сонця
                // Зменшено швидкість і заблоковано при достиженні ліміту популяції,
                // щоб уникнути експоненційного зростання продуцентів.
                if (floraCount < MAX_FLORA) {
                    entity->addEnergy(18.f * dt);  // Збільшено з 12 до 18 для стабільного цукрового балансу
                }
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
                        // Рух до цілі (Seek behavior)
                        sf::Vector2f dir = target->getPosition() - entity->getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) {
                            entity->setVelocity((dir / len) * entity->getGenome().maxSpeed);
                        }

                        // Поїдання флори: травоїдний отримує енергію, флора втрачає її
                        float collisionDist = entity->getGenome().size + target->getGenome().size;
                        if (minDist < collisionDist) {
                            entity->addEnergy(80.f);        // Травоїдний отримує енергію з рослини
                            target->addEnergy(-100.f);      // Флора втрачає енергію (биомаса зникає)
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
                        // Рух до цілі (Seek behavior)
                        sf::Vector2f dir = target->getPosition() - entity->getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) {
                            entity->setVelocity((dir / len) * entity->getGenome().maxSpeed);
                        }

                        // Поїдання травоїдного: хижак отримує менше енергії, ніж раніше,
                        // що імітує неefekтивність передачі енергії між трофічними рівнями.
                        float collisionDist = entity->getGenome().size + target->getGenome().size;
                        if (minDist < collisionDist) {
                            entity->addEnergy(80.f);       // Зменшено з 120 до 80 — менше енергії, рідше розмноження
                            target->addEnergy(-100.f);      // Травоїдний втрачає всю енергію (помер)
                        }
                    }
                }
            }
        }

        // --- КРОК Г: Розмноження (асексуальне клонування з мутацією) ---
        asexualTimer += dt;
        
        std::vector<std::unique_ptr<Entity>> newborns;
        
        if (asexualTimer >= 4.0f) {  // Зменшено з 6 до 4 секунд — флора розмножується частіше
            int newFlora = 0, newHerb = 0, newPred = 0;
            for (auto& entity : entities) {
                if (entity->isDead()) continue;

                // Обмеж розмноження за лімітами популяції (енергетична піраміда)
                Entity::OrganismType eType = entity->getType();
                if (eType == Entity::OrganismType::Flora && floraCount + newFlora >= MAX_FLORA) continue;
                if (eType == Entity::OrganismType::Herbivore && herbCount + newHerb >= MAX_HERBIVORE) continue;
                if (eType == Entity::OrganismType::Predator && predCount + newPred >= MAX_PREDATOR) continue;
                
                // Поріг енергії для розмноження: хижакам потрібно значно більше енергії,
                // оскільки на вищому трофічному рівні енергії доступно менше.
                float threshold;
                if (eType == Entity::OrganismType::Predator) {
                    threshold = std::max(300.f, entity->getGenome().maxSpeed * 2.5f);  // Вищий поріг для хижаків
                } else {
                    threshold = std::max(120.f, entity->getGenome().maxSpeed * 1.2f);
                }
                
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

                    // Оновлюємо лічильники newborn для точного ліміту всередині одного тику
                    if (eType == Entity::OrganismType::Flora) newFlora++;
                    else if (eType == Entity::OrganismType::Herbivore) newHerb++;
                    else if (eType == Entity::OrganismType::Predator) newPred++;
                }
            }
            asexualTimer = 0.f; // Таймер скидається завжди, щоб уникнути спаму
        }

        for (auto& child : newborns) {
            entities.push_back(std::move(child));
        }

        // --- КРОК Д: Очищення мертвих істот (Mark and Sweep) ---
        // Істоми, енергія яких вичерпана (до 0), видаляються з симуляції.
        // Біологічний сенс: голодова смерть — природний механізм регулювання популяції.
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
