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

// === КОНСТАНТИ БАЛАНСУ ===
// Енергетична піраміда ~100:10:1
const int MAX_FLORA = 1000;
const int MAX_HERBIVORE = 100;
const int MAX_PREDATOR = 15;

// === ПОЛЯ ЗОРУ (обмежені, не весь екран) ===
const float PREDATOR_VISION_RANGE = 150.f;         // Хижаки бачать здобич до 150px (менше за травоїдні в FLEE)
const float HERBIVORE_FOOD_VISION = 200.f;         // Травоїдні бачать флору до 200px
const float HERBIVORE_PREDATOR_VISION = 120.f;     // Травоїдні бачать хижаків до 120px (коротший радіус небезпеки)
const float HERBIVORE_MATE_VISION = 150.f;         // Травоїдні бачать партнерів до 150px

// === ЕНЕРГІЯ ТА МЕТАБОЛІЗМ ===
const float HERBIVORE_METABOLISM_MULT = 0.010f;    // Травоїдні витрачають менше енергії
const float PREDATOR_METABOLISM_MULT = 0.022f;     // Хижаки швидше голодують

// === ЕНЕРГІЯ ВІД ПОЇДАННЯ ===
const float HERBIVORE_EAT_ENERGY = 120.f;          // Збільшено з 80 -> 120
const float PREDATOR_EAT_ENERGY = 100.f;           // Зменшено з 80 -> 100 (менш поживне)

// === ШВИДКІСТІ ===
const float HERBIVORE_MAX_SPEED = 220.f;           // Збільшено з 150
const float HERBIVORE_FLEE_SPEED = 264.f;          // 20% швидше за нормальний (220 * 1.2)
const float PREDATOR_MAX_SPEED = 200.f;            // Трохи нижче за FLEE травоїдних

// === ПОРОГИ РОЗМНОЖЕННЯ ===
const float HERBIVORE_REPRO_THRESHOLD_BASE = 80.f; // Нижчий поріг для травоїдних
const float PREDATOR_REPRO_THRESHOLD_BASE = 400.f; // Вищий поріг для хижаків

// === ПОЧАТКОВІ УМОВИ ===
const int INITIAL_FLORA = 100;
const int INITIAL_HERBIVORE = 40;                  // Збільшено з 20 до 40
const int INITIAL_PREDATOR = 8;                    // Зменшено з 15 до 8
const float INITIAL_FLORA_ENERGY = 200.f;          // Збільшено з 100 -> 200 (буфер для поїдання)
const float INITIAL_HERBIVORE_ENERGY = 250.f;      // Збільшено з 150 до 250 (фора на старті)
const float INITIAL_PREDATOR_ENERGY = 300.f;       // Збільшено з 200 до 300

// === ШКОДА ВІД ПОЇДАННЯ ===
const float FLORA_EAT_DAMAGE = 50.f;               // Зменшено з 100 -> 50 (флора переживає укус)

// Допоміжна функція для розрахунку відстані
float getDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Плавна інтерполяція швидкості (lerp) для уникнення "смикання"
sf::Vector2f lerpVelocity(const sf::Vector2f& current, const sf::Vector2f& target, float factor) {
    return current + (target - current) * factor;
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Ініціалізація RNG

    // 1. Налаштування вікна
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - FSM Ecosystem");
    window.setFramerateLimit(60);
    sf::Clock clock;

    // 2. Ініціалізація SpatialGrid (світ 1024x768, комірка 100px)
    SpatialGrid grid(1024.f, 768.f, 100.f);

    // 3. Створення початкової популяції
    std::vector<std::unique_ptr<Entity>> entities;
    
    std::cout << "=== Ініціалізація FSM-екосистеми ===" << std::endl;

    // Флора (100 шт.) — продуценти першого трофічного рівня
    for (int i = 0; i < INITIAL_FLORA; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        auto flora = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(0.f, 0.f), Entity::OrganismType::Flora);
        flora->addEnergy(INITIAL_FLORA_ENERGY);
        entities.push_back(std::move(flora));
    }

    // Травоїдні (40 шт.) — споживачі другого трофічного рівня
    for (int i = 0; i < INITIAL_HERBIVORE; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        // Початкова швидкість ±150
        float vx = static_cast<float>((std::rand() % 300) - 150);
        float vy = static_cast<float>((std::rand() % 300) - 150);
        auto herb = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Herbivore);
        herb->addEnergy(INITIAL_HERBIVORE_ENERGY); // 250 енергії — фора на старті
        entities.push_back(std::move(herb));
    }

    // Хижаки (8 шт.) — споживачі третього трофічного рівня
    for (int i = 0; i < INITIAL_PREDATOR; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        // Початкова швидкість ±100
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto pred = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Predator);
        pred->addEnergy(INITIAL_PREDATOR_ENERGY); // 300 енергії
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

        // --- КРОК А: Оновлення фізики, метаболізму та FSM переходів ( внутрішні умови) ---
        for (auto& entity : entities) {
            entity->update(dt, window.getSize());
        }

        // --- КРОК Б: Оновлення SpatialGrid ---
        grid.clear();
        for (size_t i = 0; i < entities.size(); ++i) {
            grid.addEntity(entities[i]->getPosition(), i);
        }

        // Підрахунок поточної популяції для лімітів
        int floraCount = 0, herbCount = 0, predCount = 0;
        for (const auto& e : entities) {
            if (e->getType() == Entity::OrganismType::Flora) floraCount++;
            else if (e->getType() == Entity::OrganismType::Herbivore) herbCount++;
            else if (e->getType() == Entity::OrganismType::Predator) predCount++;
        }

        // --- КРОК В: FSM поведінка та харчовий ланцюг (використання SpatialGrid) ---
        for (size_t i = 0; i < entities.size(); ++i) {
            auto& entity = entities[i];
            if (entity->isDead()) continue;

            Entity::OrganismType type = entity->getType();

            if (type == Entity::OrganismType::Flora) {
                // ФОТОСИНТЕЗ — флора накопичує енергію зі сонця
                // При досягненні ліміту популяції — повільний фотосинтез для підтримки енергії
                if (floraCount < MAX_FLORA) {
                    entity->addEnergy(18.f * dt);  // Нормальний фотосинтез
                } else {
                    entity->addEnergy(5.f * dt);   // Повільний фотосинтез навіть при ліміті популяції
                }
            } 
            else if (type == Entity::OrganismType::Herbivore) {
                // === FSM ДЛЯ ТРАВОЇДНИХ ===
                // Пріоритет 1: FLEE (втеча від хижака) — найвищий пріоритет
                // Перевіряємо наявність хижаків у радіусі небезпеки
                auto nearbyPredators = grid.getNearbyEntitiesOfType(
                    entity->getPosition(), 
                    HERBIVORE_PREDATOR_VISION, 
                    Entity::OrganismType::Predator, 
                    entities
                );

                sf::Vector2f targetVelocity(0.f, 0.f);
                bool isFleeing = false;

                if (!nearbyPredators.empty()) {
                    // FLEE: втеча від найближчого хижака
                    isFleeing = true;
                    entity->setHerbivoreState(Entity::HerbivoreState::FLEE);
                    entity->setFleeTimer(2.5f); // Таймер втечі 2.5 секунди

                    // Знаходимо найближчого хижака
                    int closestPredIdx = nearbyPredators[0];
                    float minDist = getDistance(entity->getPosition(), entities[closestPredIdx]->getPosition());
                    for (int idx : nearbyPredators) {
                        float dist = getDistance(entity->getPosition(), entities[idx]->getPosition());
                        if (dist < minDist) {
                            minDist = dist;
                            closestPredIdx = idx;
                        }
                    }

                    // Рух у напрямку, протилежному до хижака
                    sf::Vector2f dir = entity->getPosition() - entities[closestPredIdx]->getPosition();
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (len > 0.f) {
                        targetVelocity = (dir / len) * HERBIVORE_FLEE_SPEED; // Адреналін: +20% швидкості
                    }
                } 
                else {
                    // FLEE не потрібний — перевіряємо внутрішній стан (установлений в Entity::update на основі енергії)
                    Entity::HerbivoreState state = entity->getHerbivoreState();

                    if (state == Entity::HerbivoreState::SEEK_FOOD) {
                        // ПОШУК ЇЖІ: шукаємо найближчу флору
                        auto targets = grid.getNearbyEntitiesOfType(
                            entity->getPosition(), 
                            HERBIVORE_FOOD_VISION, 
                            Entity::OrganismType::Flora, 
                            entities
                        );
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
                                    targetVelocity = (dir / len) * HERBIVORE_MAX_SPEED;
                                }
                            }
                        }
                    } 
                    else if (state == Entity::HerbivoreState::SEEK_MATE) {
                        // ПОШУК ПАРТНЕРА: рухаємося до центру маси сусідніх травоїдних
                        auto targets = grid.getNearbyEntitiesOfType(
                            entity->getPosition(), 
                            HERBIVORE_MATE_VISION, 
                            Entity::OrganismType::Herbivore, 
                            entities
                        );
                        if (!targets.empty()) {
                            // Середня позиція сусідніх травоїдних
                            sf::Vector2f avgPos(0.f, 0.f);
                            int count = 0;
                            for (int idx : targets) {
                                avgPos += entities[idx]->getPosition();
                                count++;
                            }
                            if (count > 0) {
                                avgPos /= static_cast<float>(count);
                                sf::Vector2f dir = avgPos - entity->getPosition();
                                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                                if (len > 0.f) {
                                    targetVelocity = (dir / len) * HERBIVORE_MAX_SPEED;
                                }
                            }
                        }
                    } 
                    else { // WANDER
                        // БЛУКАННЯ: використовуємо збережений напрямок
                        sf::Vector2f wanderDir = entity->getWanderDirection();
                        if (std::abs(wanderDir.x) > 0.001f || std::abs(wanderDir.y) > 0.001f) {
                            targetVelocity = wanderDir * (HERBIVORE_MAX_SPEED * 0.5f); // Повільніше при блуканні
                        }
                    }
                }

                // === Плавна інтерполяція швидкості (lerp) ===
                sf::Vector2f currentVel = entity->getVelocity();
                sf::Vector2f newVel = lerpVelocity(currentVel, targetVelocity, 0.15f);
                entity->setVelocity(newVel);

                // === ПОЇДАННЯ ФЛОРИ (тепер і в FLEE, щоб травоїдні не голодували) ===
                auto foodTargets = grid.getNearbyEntitiesOfType(
                    entity->getPosition(), 
                    entity->getGenome().size + 20.f, // Невеликий буфер для зіткнення
                    Entity::OrganismType::Flora, 
                    entities
                );
                for (int idx : foodTargets) {
                    auto& target = entities[idx];
                    if (!target->isDead()) {
                        float dist = getDistance(entity->getPosition(), target->getPosition());
                        float collisionDist = entity->getGenome().size + target->getGenome().size;
                        if (dist < collisionDist) {
                            entity->addEnergy(HERBIVORE_EAT_ENERGY);   // +120 енергії
                            target->addEnergy(-FLORA_EAT_DAMAGE);      // -50, флора переживає
                        }
                    }
                }
            } 
            else if (type == Entity::OrganismType::Predator) {
                // === FSM ДЛЯ ХИЖАКІВ ===
                // Пріоритет 1: SEEK_PREY (переслідування здобичі)
                
                auto nearbyPrey = grid.getNearbyEntitiesOfType(
                    entity->getPosition(), 
                    PREDATOR_VISION_RANGE, 
                    Entity::OrganismType::Herbivore, 
                    entities
                );

                sf::Vector2f targetVelocity(0.f, 0.f);
                bool isHunting = false;

                if (!nearbyPrey.empty()) {
                    // SEEK_PREY: переслідування найближчої травоїдної
                    isHunting = true;
                    entity->setPredatorState(Entity::PredatorState::SEEK_PREY);

                    int closestIdx = nearbyPrey[0];
                    float minDist = getDistance(entity->getPosition(), entities[closestIdx]->getPosition());
                    for (int idx : nearbyPrey) {
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
                            targetVelocity = (dir / len) * PREDATOR_MAX_SPEED;
                        }
                    }
                } 
                else {
                    // WANDER: повільне блукання для економії енергії
                    entity->setPredatorState(Entity::PredatorState::WANDER);
                    sf::Vector2f wanderDir = entity->getWanderDirection();
                    if (std::abs(wanderDir.x) > 0.001f || std::abs(wanderDir.y) > 0.001f) {
                        targetVelocity = wanderDir * (PREDATOR_MAX_SPEED * 0.3f); // Дуже повільно при блуканні
                    }
                }

                // Плавна інтерполяція швидкості
                sf::Vector2f currentVel = entity->getVelocity();
                sf::Vector2f newVel = lerpVelocity(currentVel, targetVelocity, 0.15f);
                entity->setVelocity(newVel);

                // === ПОЇДАННЯ ТРАВОЇДНОЇ ===
                if (isHunting) {
                    auto preyTargets = grid.getNearbyEntitiesOfType(
                        entity->getPosition(), 
                        entity->getGenome().size + 20.f,
                        Entity::OrganismType::Herbivore, 
                        entities
                    );
                    for (int idx : preyTargets) {
                        auto& target = entities[idx];
                        if (!target->isDead()) {
                            float dist = getDistance(entity->getPosition(), target->getPosition());
                            float collisionDist = entity->getGenome().size + target->getGenome().size;
                            if (dist < collisionDist) {
                                entity->addEnergy(130.f);      // +130 енергії (було +100, NET ZERO виправлено)
                                target->addEnergy(-100.f);     // Травоїдна вмирає
                            }
                        }
                    }
                }
            }
        }

        // --- КРОК Г: РОЗМНОЖЕННЯ (асексуальне клонування з мутацією) ---
        asexualTimer += dt;
        
        std::vector<std::unique_ptr<Entity>> newborns;
        
        if (asexualTimer >= 4.0f) {
            int newFlora = 0, newHerb = 0, newPred = 0;
            for (auto& entity : entities) {
                if (entity->isDead()) continue;

                // Обмеж розмноження за лімітами популяції
                Entity::OrganismType eType = entity->getType();
                if (eType == Entity::OrganismType::Flora && floraCount + newFlora >= MAX_FLORA) continue;
                if (eType == Entity::OrganismType::Herbivore && herbCount + newHerb >= MAX_HERBIVORE) continue;
                if (eType == Entity::OrganismType::Predator && predCount + newPred >= MAX_PREDATOR) continue;
                
                // Поріг енергії для розмноження за типом
                float threshold;
                if (eType == Entity::OrganismType::Herbivore) {
                    // Травоїдні розмножуються частіше (нижчий поріг)
                    threshold = std::max(HERBIVORE_REPRO_THRESHOLD_BASE, entity->getGenome().maxSpeed * 0.5f);
                } else if (eType == Entity::OrganismType::Predator) {
                    // Хижаки розмножуються рідше (нижчий поріг, бо отримують більше енергії від полювання)
                    threshold = std::max(250.f, entity->getGenome().maxSpeed * 1.2f);
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
        entities.erase(
            std::remove_if(entities.begin(), entities.end(), 
                [](const std::unique_ptr<Entity>& e) { return e->isDead(); }),
            entities.end()
        );

        // --- КРОК Е: РЕНДЕРИНГ (шари: Флора -> Травоїдні -> Хижаки) ---
        window.clear(sf::Color::Black); // Чорний фон для максимального контрасту

        // 1. Спочатку малюємо флору (фоновий шар, напівпрозора)
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Flora) {
                entity->draw(window);
            }
        }

        // 2. Потім травоїдні (середній шар, яскравіші за флору)
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Herbivore) {
                entity->draw(window);
            }
        }

        // 3. Останні хижаки (верхній шар, завжди поверх усіх)
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Predator) {
                entity->draw(window);
            }
        }
        window.display();

        // --- Відладочний вивід статистики кожну секунду ---
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