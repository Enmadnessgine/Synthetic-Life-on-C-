#include "Entity.h"
#include <cmath>

Entity::Entity(sf::Vector2f position, sf::Vector2f velocity, OrganismType type)
    : position_(position), velocity_(velocity), energy_(100.f), type_(type)
{
    shape_.setPosition(position_);

    switch (type) {
        case OrganismType::Flora:
            // Флора: маленькі напівпрозорі зелені кружечки (фоновий шар)
            // Радіус 9, прозорість 210 — добре видно на екрані.
            // Флора нерухома, не має поля зору.
            shape_.setRadius(9.f);
            shape_.setFillColor(sf::Color(0, 255, 0, 210));
            shape_.setOutlineThickness(0.f);
            genome_.maxSpeed = 0.f;
            genome_.visionRange = 0.f;
            break;
        case OrganismType::Herbivore:
            // Травоїдні: великі яскраві жовті/золоті кружечки з помаранчевим контуром
            // Радіус 12, прозорість 220 — середній шар візуальної ієрархії.
            // Вища максимальна швидкість (220) щоб врятовуватися від хижаків.
            shape_.setRadius(12.f);
            shape_.setFillColor(sf::Color(255, 215, 0, 220));
            shape_.setOutlineColor(sf::Color(255, 140, 0, 220));
            shape_.setOutlineThickness(2.f);
            genome_.maxSpeed = 220.f;   // Збільшено з 150 до 220 для врятовування
            genome_.visionRange = 200.f; // Поле зору для пошуку флори
            break;
        case OrganismType::Predator:
            // Хижаки: великі яскраво-червоні кружечки з жовтим контуром (верхній шар)
            // Радіус 18, непрозорі — завжди видно поверх усіх інших істот.
            // Швидкість 200 — трохи нижче за травоїдних у режимі втечі (260).
            shape_.setRadius(18.f);
            shape_.setFillColor(sf::Color(255, 0, 0, 255));
            shape_.setOutlineColor(sf::Color(255, 255, 0, 255));
            shape_.setOutlineThickness(3.f);
            genome_.maxSpeed = 200.f;   // Трохи нижче за FLEE-швидкість травоїдних
            genome_.visionRange = 250.f; // Поле зору для пошуку здобичі (не весь екран!)
            break;
    }
}

void Entity::setHerbivoreState(HerbivoreState state) {
    if (currentHerbivoreState != state) {
        currentHerbivoreState = state;
        // Відладочний вивід при зміні стану (для викладача)
        // std::cout << "[FSM] Herbivore state changed to: " << static_cast<int>(state) << std::endl;
    }
}

void Entity::setPredatorState(PredatorState state) {
    if (currentPredatorState != state) {
        currentPredatorState = state;
        // Відладочний вивід при зміні стану (для викладача)
        // std::cout << "[FSM] Predator state changed to: " << static_cast<int>(state) << std::endl;
    }
}

void Entity::update(float dt, const sf::Vector2u& windowSize) {
    // --- КРОК 1: ІНТЕГРАЦІЯ ПОЗИЦІЇ (критично для руху) ---
    position_ += velocity_ * dt;

    // --- КРОК 2: МЕНЕДЖЕР СТАНІВ (FSM) ---
    // Перевірка умов переходу станів на основі внутрішніх показників (енергія)
    // ФLEE обробляється в main.cpp (потрібен доступ до SpatialGrid і позицій хижаків)

    if (type_ == OrganismType::Herbivore) {
        // Пріоритети для травоїдних: FLEE > SEEK_MATE > SEEK_FOOD > WANDER
        // FLEE обробляється в main.cpp. Тут обробляємо решта на основі енергії.
        float energyPct = energy_ / 250.f; // Початкова енергія травоїдного = 250

        if (energyPct >= 0.80f) {
            // Достатньо енергії для розмноження (> 80%)
            if (currentHerbivoreState != HerbivoreState::SEEK_MATE) {
                setHerbivoreState(HerbivoreState::SEEK_MATE);
            }
        }
        else if (energyPct < 0.60f) {
            // Низька енергія — шукаємо їжу (< 60%)
            if (currentHerbivoreState != HerbivoreState::SEEK_FOOD) {
                setHerbivoreState(HerbivoreState::SEEK_FOOD);
            }
        }
        else {
            // Нормальна енергія (60-80%) — блукання, якщо не в іншому стані
            if (currentHerbivoreState == HerbivoreState::WANDER) {
                // Залишаємося в WANDER
            }
            else if (currentHerbivoreState != HerbivoreState::WANDER &&
                     currentHerbivoreState != HerbivoreState::FLEE) {
                setHerbivoreState(HerbivoreState::WANDER);
            }
        }

        // WANDER behavior — періодична зміна напрямку кожні 2-3 секунди
        if (currentHerbivoreState == HerbivoreState::WANDER) {
            wanderTimer += dt;
            if (wanderTimer >= 2.5f) {
                wanderTimer = 0.f;
                // Випадковий напрямок руху
                float angle = static_cast<float>(std::rand()) / RAND_MAX * 2.f * 3.14159265f;
                wanderDirection = sf::Vector2f(std::cos(angle), std::sin(angle));
            }
        }
    }
    else if (type_ == OrganismType::Predator) {
        // Пріоритети для хижаків: SEEK_PREY > WANDER
        // SEEK_PREY обробляється в main.cpp (потрібен доступ до SpatialGrid)

        // WANDER behavior для хижаків — змінюють напрямок рідше (економія енергії)
        if (currentPredatorState == PredatorState::WANDER) {
            wanderTimer += dt;
            if (wanderTimer >= 4.0f) {
                wanderTimer = 0.f;
                float angle = static_cast<float>(std::rand()) / RAND_MAX * 2.f * 3.14159265f;
                wanderDirection = sf::Vector2f(std::cos(angle), std::sin(angle));
            }
        }
    }

    // --- FLEE TIMER: автоматичний вихід із стану втечі ---
    if (type_ == OrganismType::Herbivore && currentHerbivoreState == HerbivoreState::FLEE) {
        if (fleeTimer > 0.f) {
            fleeTimer -= dt;
            if (fleeTimer <= 0.f) {
                // Час втечі вичерпано — повертаємось до стану за енергією
                float energyPct = energy_ / 250.f;
                if (energyPct < 0.60f) {
                    setHerbivoreState(HerbivoreState::SEEK_FOOD);
                } else if (energyPct >= 0.80f) {
                    setHerbivoreState(HerbivoreState::SEEK_MATE);
                } else {
                    setHerbivoreState(HerbivoreState::WANDER);
                }
            }
        }
    }
    else if (type_ == OrganismType::Predator) {
        // Пріоритети для хижаків: SEEK_PREY > WANDER
        // SEEK_PREY обробляється в main.cpp (потрібен доступ до SpatialGrid)

        // WANDER behavior для хижаків — змінюють напрямок рідше (економія енергії)
        if (currentPredatorState == PredatorState::WANDER) {
            wanderTimer += dt;
            if (wanderTimer >= 4.0f) {
                wanderTimer = 0.f;
                float angle = static_cast<float>(std::rand()) / RAND_MAX * 2.f * 3.14159265f;
                wanderDirection = sf::Vector2f(std::cos(angle), std::sin(angle));
            }
        }
    }

    // --- КРОК 3: МЕТАБОЛІЗМ ---
    // Травоїдні витрачають менше енергії (зменшено з 0.015 до 0.010)
    // Хижаки витрачають більше енергії (збільшено з 0.018 до 0.022)
    float metabolism_multiplier;
    if (type_ == OrganismType::Predator) {
        metabolism_multiplier = 0.022f;  // Хижаки швидше голодують
    } else if (type_ == OrganismType::Herbivore) {
        metabolism_multiplier = 0.010f;  // Травоїдні витрачають менше енергії
    } else {
        metabolism_multiplier = 0.0f;    // Флора не має метаболізму (фотосинтез у main.cpp)
    }
    float metabolism_cost = genome_.maxSpeed * metabolism_multiplier;
    energy_ -= metabolism_cost * dt;

    // --- КРОК 4: Тороїдна топологія (wrap-around) ---
    // Коли клітина виходить за край екрану, вона з'являється з протилежного боку
    if (position_.x < 0.f) {
        position_.x += static_cast<float>(windowSize.x);
    } else if (position_.x >= static_cast<float>(windowSize.x)) {
        position_.x -= static_cast<float>(windowSize.x);
    }
    if (position_.y < 0.f) {
        position_.y += static_cast<float>(windowSize.y);
    } else if (position_.y >= static_cast<float>(windowSize.y)) {
        position_.y -= static_cast<float>(windowSize.y);
    }

    // --- КРОК 5: Візуальний фідбек ---
    // Прозорість залежить від енергії, базовий колір типу зберігається
    // Кожен тип має власний дільник для alpha (макс. енергія цього типу)
    float alpha_divisor;
    if (type_ == OrganismType::Flora) {
        alpha_divisor = 100.f;   // Флора макс. енергія = 100
    } else if (type_ == OrganismType::Herbivore) {
        alpha_divisor = 250.f;   // Травоїдний макс. енергія = 250
    } else {
        alpha_divisor = 300.f;   // Хижак макс. енергія = 300
    }
    float alpha = energy_ / alpha_divisor;
    if (alpha < 0.3f) alpha = 0.3f; // Флора ніколи не стає занадто прозорою
    sf::Color baseColor = shape_.getFillColor();
    shape_.setFillColor(sf::Color(baseColor.r, baseColor.g, baseColor.b,
        static_cast<unsigned char>(baseColor.a * alpha)));

    shape_.setPosition(position_);
}

void Entity::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
}
