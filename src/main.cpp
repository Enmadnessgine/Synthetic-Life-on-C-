#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <SFML/Graphics.hpp>

#include "entities/Entity.h"
#include "spatial/SpatialGrid.h"

// === CONSTANTS ===
const int MAX_FLORA = 1000;
const int MAX_HERBIVORE = 100;
const int MAX_PREDATOR = 15;

const float PREDATOR_VISION_RANGE = 150.f;
const float HERBIVORE_FOOD_VISION = 200.f;
const float HERBIVORE_PREDATOR_VISION = 120.f;
const float HERBIVORE_MATE_VISION = 150.f;

const float HERBIVORE_METABOLISM_MULT = 0.010f;
const float PREDATOR_METABOLISM_MULT = 0.022f;

const float HERBIVORE_EAT_ENERGY = 120.f;
const float PREDATOR_EAT_ENERGY = 130.f;

const float HERBIVORE_MAX_SPEED = 220.f;
const float HERBIVORE_FLEE_SPEED = 264.f;
const float PREDATOR_MAX_SPEED = 200.f;

const float HERBIVORE_REPRO_THRESHOLD_BASE = 80.f;
const float PREDATOR_REPRO_THRESHOLD_BASE = 250.f;

const int INITIAL_FLORA = 100;
const int INITIAL_HERBIVORE = 40;
const int INITIAL_PREDATOR = 8;
const float INITIAL_FLORA_ENERGY = 200.f;
const float INITIAL_HERBIVORE_ENERGY = 250.f;
const float INITIAL_PREDATOR_ENERGY = 300.f;
const float FLORA_EAT_DAMAGE = 50.f;

// Distance function
float getDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

sf::Vector2f lerpVelocity(const sf::Vector2f& current, const sf::Vector2f& target, float factor) {
    return current + (target - current) * factor;
}

// === UI State ===
struct SimParams {
    int initialFlora = 100;
    int initialHerbivore = 40;
    int initialPredator = 8;
    
    float floraPhotosynthesis = 18.0f;
    float herbivoreMetabolism = 0.010f;
    float predatorMetabolism = 0.022f;
    
    float herbivoreFoodVision = 200.0f;
    float herbivorePredatorVision = 120.0f;
    float herbivoreMateVision = 150.0f;
    float predatorVision = 150.0f;
    
    float timeScale = 1.0f;
    bool isPaused = false;
};

struct UIState {
    sf::Font font;
    bool fontLoaded = false;
    bool showPopulationStats = true;
    bool showControlPanel = true;
    bool showGraphsPanel = true;
    bool showExperimentPanel = true;
    bool showLoggingPanel = true;
    bool isLogging = false;
    
    std::vector<float> floraHistory;
    std::vector<float> herbivoreHistory;
    std::vector<float> predatorHistory;
    int maxHistoryPoints = 600;
    
    std::ofstream logFile;
    float logTimer = 0.0f;
    
    SimParams params;
};

bool loadFont(sf::Font& font) {
    if (font.openFromFile("C:/Windows/Fonts/arial.ttf")) return true;
    if (font.openFromFile("C:/Windows/Fonts/segoeui.ttf")) return true;
    return false;
}

void drawPanel(sf::RenderWindow& window, float x, float y, float w, float h, const sf::Color& bgColor) {
    sf::RectangleShape panel(sf::Vector2f(w, h));
    panel.setPosition(sf::Vector2f(x, y));
    panel.setFillColor(bgColor);
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(1.f);
    window.draw(panel);
}

void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& text, float x, float y, unsigned int size, sf::Color color) {
    sf::Text txt(font, text, size);
    txt.setFillColor(color);
    txt.setPosition(sf::Vector2f(x, y));
    window.draw(txt);
}

void drawButton(sf::RenderWindow& window, const sf::Font& font, const std::string& text, float x, float y, float w, float h, bool hovered, bool pressed) {
    sf::RectangleShape btn(sf::Vector2f(w, h));
    btn.setPosition(sf::Vector2f(x, y));
    btn.setFillColor(pressed ? sf::Color(60, 60, 80) : (hovered ? sf::Color(80, 80, 100) : sf::Color(60, 60, 80)));
    btn.setOutlineColor(sf::Color(150, 150, 150));
    btn.setOutlineThickness(1.f);
    window.draw(btn);
    
    sf::Text btnText(font, text, 13);
    btnText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = btnText.getLocalBounds();
    btnText.setOrigin(sf::Vector2f(textBounds.position.x + textBounds.size.x / 2.f, textBounds.position.y + textBounds.size.y / 2.f));
    btnText.setPosition(sf::Vector2f(x + w / 2.f, y + h / 2.f - 2.f));
    window.draw(btnText);
}

bool isMouseInRect(const sf::Vector2f& mousePos, float x, float y, float w, float h) {
    return mousePos.x >= x && mousePos.x <= x + w && mousePos.y >= y && mousePos.y <= y + h;
}

void drawGraphLine(sf::RenderWindow& window, const std::vector<float>& history, float x, float y, float w, float h, sf::Color color) {
    if (history.size() < 2) return;
    
    sf::VertexArray line(sf::PrimitiveType::LineStrip, history.size());
    for (size_t i = 0; i < history.size(); ++i) {
        float px = x + static_cast<float>(i) / (history.size() - 1) * w;
        float py = y + h - (history[i] / 1000.f) * h;
        if (py < y) py = y;
        if (py > y + h) py = y + h;
        line[i].position = sf::Vector2f(px, py);
        line[i].color = color;
    }
    window.draw(line);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 1. Window setup
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Artificial Life - FSM Ecosystem with UI");
    window.setFramerateLimit(60);
    sf::Clock clock;

    // UI State
    UIState uiState;
    uiState.fontLoaded = loadFont(uiState.font);
    uiState.logFile.open("simulation_log.csv");
    if (uiState.logFile.is_open()) {
        uiState.logFile << "Time,Flora,Herbivore,Predator\n";
    }
    uiState.isLogging = true;

    // SpatialGrid
    SpatialGrid grid(1024.f, 768.f, 100.f);

    // Initial population
    std::vector<std::unique_ptr<Entity>> entities;
    
    std::cout << "=== Initializing FSM Ecosystem with UI ===" << std::endl;

    for (int i = 0; i < INITIAL_FLORA; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        auto flora = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(0.f, 0.f), Entity::OrganismType::Flora);
        flora->addEnergy(INITIAL_FLORA_ENERGY);
        entities.push_back(std::move(flora));
    }

    for (int i = 0; i < INITIAL_HERBIVORE; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 300) - 150);
        float vy = static_cast<float>((std::rand() % 300) - 150);
        auto herb = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Herbivore);
        herb->addEnergy(INITIAL_HERBIVORE_ENERGY);
        entities.push_back(std::move(herb));
    }

    for (int i = 0; i < INITIAL_PREDATOR; ++i) {
        float x = static_cast<float>(std::rand() % 1024);
        float y = static_cast<float>(std::rand() % 768);
        float vx = static_cast<float>((std::rand() % 200) - 100);
        float vy = static_cast<float>((std::rand() % 200) - 100);
        auto pred = std::make_unique<Entity>(sf::Vector2f(x, y), sf::Vector2f(vx, vy), Entity::OrganismType::Predator);
        pred->addEnergy(INITIAL_PREDATOR_ENERGY);
        entities.push_back(std::move(pred));
    }
    std::cout << "Created entities. Total: " << entities.size() << "\n" << std::endl;

    float asexualTimer = 0.f;
    float simulationTime = 0.0f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        // Input handling
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        bool leftPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        // Event handling (SFML 3.0 API)
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Get UI parameters
        const SimParams& params = uiState.params;
        float timeScale = uiState.params.timeScale;
        bool isPaused = uiState.params.isPaused;

        // --- Step A: Physics, metabolism, FSM transitions ---
        if (!isPaused) {
            float scaledDt = dt * timeScale;
            
            for (auto& entity : entities) {
                entity->update(scaledDt, window.getSize());
            }
        }

        // --- Step B: Update SpatialGrid ---
        grid.clear();
        for (size_t i = 0; i < entities.size(); ++i) {
            grid.addEntity(entities[i]->getPosition(), i);
        }

        // Population counts for limits
        int floraCount = 0, herbCount = 0, predCount = 0;
        for (const auto& e : entities) {
            if (e->getType() == Entity::OrganismType::Flora) floraCount++;
            else if (e->getType() == Entity::OrganismType::Herbivore) herbCount++;
            else if (e->getType() == Entity::OrganismType::Predator) predCount++;
        }

        // --- Step C: FSM behavior and food chain ---
        if (!isPaused) {
            float scaledDt = dt * timeScale;
            
            for (size_t i = 0; i < entities.size(); ++i) {
                auto& entity = entities[i];
                if (entity->isDead()) continue;

                Entity::OrganismType type = entity->getType();

                if (type == Entity::OrganismType::Flora) {
                    // Photosynthesis
                    if (floraCount < MAX_FLORA) {
                        entity->addEnergy(uiState.params.floraPhotosynthesis * scaledDt);
                    } else {
                        entity->addEnergy(5.f * scaledDt); // Slow photosynthesis at cap
                    }
                } 
                else if (type == Entity::OrganismType::Herbivore) {
                    // FSM for Herbivores
                    auto nearbyPredators = grid.getNearbyEntitiesOfType(
                        entity->getPosition(), 
                        uiState.params.herbivorePredatorVision, 
                        Entity::OrganismType::Predator, 
                        entities
                    );

                    sf::Vector2f targetVelocity(0.f, 0.f);
                    bool isFleeing = false;

                    if (!nearbyPredators.empty()) {
                        isFleeing = true;
                        entity->setHerbivoreState(Entity::HerbivoreState::FLEE);
                        entity->setFleeTimer(2.5f);

                        int closestPredIdx = nearbyPredators[0];
                        float minDist = getDistance(entity->getPosition(), entities[closestPredIdx]->getPosition());
                        for (int idx : nearbyPredators) {
                            float dist = getDistance(entity->getPosition(), entities[idx]->getPosition());
                            if (dist < minDist) {
                                minDist = dist;
                                closestPredIdx = idx;
                            }
                        }

                        sf::Vector2f dir = entity->getPosition() - entities[closestPredIdx]->getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) {
                            targetVelocity = (dir / len) * HERBIVORE_FLEE_SPEED;
                        }
                    } 
                    else {
                        Entity::HerbivoreState state = entity->getHerbivoreState();

                        if (state == Entity::HerbivoreState::SEEK_FOOD) {
                            auto targets = grid.getNearbyEntitiesOfType(
                                entity->getPosition(), 
                                uiState.params.herbivoreFoodVision, 
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
                            auto targets = grid.getNearbyEntitiesOfType(
                                entity->getPosition(), 
                                uiState.params.herbivoreMateVision, 
                                Entity::OrganismType::Herbivore, 
                                entities
                            );
                            if (!targets.empty()) {
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
                            sf::Vector2f wanderDir = entity->getWanderDirection();
                            if (std::abs(wanderDir.x) > 0.001f || std::abs(wanderDir.y) > 0.001f) {
                                targetVelocity = wanderDir * (HERBIVORE_MAX_SPEED * 0.5f);
                            }
                        }
                    }

                    // Smooth velocity interpolation
                    sf::Vector2f currentVel = entity->getVelocity();
                    sf::Vector2f newVel = lerpVelocity(currentVel, targetVelocity, 0.15f);
                    entity->setVelocity(newVel);

                    // Eating flora (even during FLEE)
                    auto foodTargets = grid.getNearbyEntitiesOfType(
                        entity->getPosition(), 
                        entity->getGenome().size + 20.f,
                        Entity::OrganismType::Flora, 
                        entities
                    );
                    for (int idx : foodTargets) {
                        auto& target = entities[idx];
                        if (!target->isDead()) {
                            float dist = getDistance(entity->getPosition(), target->getPosition());
                            float collisionDist = entity->getGenome().size + target->getGenome().size;
                            if (dist < collisionDist) {
                                entity->addEnergy(HERBIVORE_EAT_ENERGY);
                                target->addEnergy(-FLORA_EAT_DAMAGE);
                            }
                        }
                    }
                } 
                else if (type == Entity::OrganismType::Predator) {
                    // FSM for Predators
                    auto nearbyPrey = grid.getNearbyEntitiesOfType(
                        entity->getPosition(), 
                        uiState.params.predatorVision, 
                        Entity::OrganismType::Herbivore, 
                        entities
                    );

                    sf::Vector2f targetVelocity(0.f, 0.f);
                    bool isHunting = false;

                    if (!nearbyPrey.empty()) {
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
                        entity->setPredatorState(Entity::PredatorState::WANDER);
                        sf::Vector2f wanderDir = entity->getWanderDirection();
                        if (std::abs(wanderDir.x) > 0.001f || std::abs(wanderDir.y) > 0.001f) {
                            targetVelocity = wanderDir * (PREDATOR_MAX_SPEED * 0.3f);
                        }
                    }

                    // Smooth velocity interpolation
                    sf::Vector2f currentVel = entity->getVelocity();
                    sf::Vector2f newVel = lerpVelocity(currentVel, targetVelocity, 0.15f);
                    entity->setVelocity(newVel);

                    // Eating herbivore
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
                                    entity->addEnergy(PREDATOR_EAT_ENERGY);
                                    target->addEnergy(-100.f);
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Step D: Reproduction ---
        if (!isPaused) {
            float scaledDt = dt * timeScale;
            asexualTimer += scaledDt;
            
            std::vector<std::unique_ptr<Entity>> newborns;
            
            if (asexualTimer >= 4.0f) {
                int newFlora = 0, newHerb = 0, newPred = 0;
                for (auto& entity : entities) {
                    if (entity->isDead()) continue;

                    Entity::OrganismType eType = entity->getType();
                    if (eType == Entity::OrganismType::Flora && floraCount + newFlora >= MAX_FLORA) continue;
                    if (eType == Entity::OrganismType::Herbivore && herbCount + newHerb >= MAX_HERBIVORE) continue;
                    if (eType == Entity::OrganismType::Predator && predCount + newPred >= MAX_PREDATOR) continue;
                    
                    float threshold;
                    if (eType == Entity::OrganismType::Herbivore) {
                        threshold = std::max(HERBIVORE_REPRO_THRESHOLD_BASE, entity->getGenome().maxSpeed * 0.5f);
                    } else if (eType == Entity::OrganismType::Predator) {
                        threshold = std::max(PREDATOR_REPRO_THRESHOLD_BASE, entity->getGenome().maxSpeed * 1.2f);
                    } else {
                        threshold = std::max(120.f, entity->getGenome().maxSpeed * 1.2f);
                    }
                    
                    if (entity->getEnergy() >= threshold) {
                        float energyToPass = entity->getEnergy() * 0.4f;
                        entity->addEnergy(-energyToPass);

                        Genome childGenome = entity->getGenome();
                        childGenome.maxSpeed += static_cast<float>((std::rand() % 21) - 10);
                        childGenome.size += static_cast<float>((std::rand() % 5) - 2);
                        
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

                        if (eType == Entity::OrganismType::Flora) newFlora++;
                        else if (eType == Entity::OrganismType::Herbivore) newHerb++;
                        else if (eType == Entity::OrganismType::Predator) newPred++;
                    }
                }
                asexualTimer = 0.f;
            }

            for (auto& child : newborns) {
                entities.push_back(std::move(child));
            }
        }

        // --- Step E: Cleanup dead entities ---
        entities.erase(
            std::remove_if(entities.begin(), entities.end(), 
                [](const std::unique_ptr<Entity>& e) { return e->isDead(); }),
            entities.end()
        );

        // Update UI state (history, logging)
        int flora = 0, herb = 0, pred = 0;
        for (const auto& e : entities) {
            if (e->getType() == Entity::OrganismType::Flora) flora++;
            else if (e->getType() == Entity::OrganismType::Herbivore) herb++;
            else if (e->getType() == Entity::OrganismType::Predator) pred++;
        }
        
        uiState.floraHistory.push_back(static_cast<float>(flora));
        uiState.herbivoreHistory.push_back(static_cast<float>(herb));
        uiState.predatorHistory.push_back(static_cast<float>(pred));
        
        if (static_cast<int>(uiState.floraHistory.size()) > uiState.maxHistoryPoints) {
            uiState.floraHistory.erase(uiState.floraHistory.begin());
            uiState.herbivoreHistory.erase(uiState.herbivoreHistory.begin());
            uiState.predatorHistory.erase(uiState.predatorHistory.begin());
        }

        uiState.logTimer += dt;
        if (uiState.logTimer >= 1.0f) {
            uiState.logTimer = 0.0f;
            if (uiState.isLogging && uiState.logFile.is_open()) {
                uiState.logFile << flora << "," << herb << "," << pred << "\n";
                uiState.logFile.flush();
            }
        }

        // --- Rendering ---
        window.clear(sf::Color::Black);

        // Layered rendering: Flora -> Herbivores -> Predators
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Flora) {
                entity->draw(window);
            }
        }
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Herbivore) {
                entity->draw(window);
            }
        }
        for (const auto& entity : entities) {
            if (entity->getType() == Entity::OrganismType::Predator) {
                entity->draw(window);
            }
        }

        // Draw UI panels
        sf::Font font = uiState.font;
        if (uiState.fontLoaded) {
            // Menu bar
            sf::RectangleShape menuBar(sf::Vector2f(1024, 25));
            menuBar.setFillColor(sf::Color(40, 40, 50));
            window.draw(menuBar);
            
            sf::Text titleText(uiState.font, "Artificial Life - FSM Ecosystem", 14);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition(sf::Vector2f(10, 4));
            window.draw(titleText);
            
            // Menu items
            sf::Text pauseText(uiState.font, isPaused ? "[P] PAUSED" : "[P] Running", 12);
            pauseText.setFillColor(isPaused ? sf::Color(255, 100, 100) : sf::Color(100, 255, 100));
            pauseText.setPosition(sf::Vector2f(200, 4));
            window.draw(pauseText);
            
            sf::Text timeText(uiState.font, "[+/-] Time: " + std::to_string(uiState.params.timeScale).substr(0, 4) + "x", 12);
            timeText.setFillColor(sf::Color(200, 200, 200));
            timeText.setPosition(sf::Vector2f(320, 4));
            window.draw(timeText);
            
            sf::Text logText(uiState.font, "[L] Logging: " + std::string(uiState.isLogging ? "ON" : "OFF"), 12);
            logText.setFillColor(uiState.isLogging ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
            logText.setPosition(sf::Vector2f(500, 4));
            window.draw(logText);
        }

        window.display();

        // Console statistics every second
        static int frameCount = 0;
        if (++frameCount % 60 == 0) {
            int flora = 0, herb = 0, pred = 0;
            for (const auto& e : entities) {
                if (e->getType() == Entity::OrganismType::Flora) flora++;
                else if (e->getType() == Entity::OrganismType::Herbivore) herb++;
                else if (e->getType() == Entity::OrganismType::Predator) pred++;
            }
            std::cout << "Stats -> Flora: " << flora 
                      << " | Herbivore: " << herb 
                      << " | Predator: " << pred << std::endl;
        }
    }

    return 0;
}