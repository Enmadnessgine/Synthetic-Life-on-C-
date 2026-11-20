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

// Overloaded drawButton without hover/pressed for simple static buttons
void drawButton(sf::RenderWindow& window, const sf::Font& font, const std::string& text, float x, float y, float w, float h) {
    drawButton(window, font, text, x, y, w, h, false, false);
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
        window.clear(sf::Color(20, 20, 20)); // Dark background

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

        // --- UI Panels (drawn over entities, under menu bar) ---
        if (uiState.fontLoaded) {
            // ============================================================
            // LEFT TOP: Control Panel (Settings)
            // ============================================================
            drawPanel(window, 10.f, 40.f, 280.f, 520.f, sf::Color(40, 40, 50, 230));

            float cx = 20.f, cy = 50.f;
            drawText(window, uiState.font, "SIMULATION CONTROL", cx, cy, 16, sf::Color(200, 200, 255));
            cy += 25;

            // Time Control
            drawText(window, uiState.font, "TIME CONTROL", cx, cy, 14, sf::Color(200, 200, 255));
            cy += 20;
            drawText(window, uiState.font, "Time Scale: " + std::to_string(uiState.params.timeScale).substr(0, 4) + "x", cx, cy, 13, sf::Color::White);
            cy += 20;
            // Time scale slider visual
            drawPanel(window, cx, cy, 200.f, 10.f, sf::Color(50, 50, 70));
            float sliderPos = cx + (uiState.params.timeScale - 0.1f) / (3.0f - 0.1f) * 200.f;
            sf::RectangleShape slider(sf::Vector2f(10.f, 14.f));
            slider.setPosition(sf::Vector2f(sliderPos, cy - 2.f));
            slider.setFillColor(sf::Color(100, 200, 255));
            window.draw(slider);
            cy += 20;
            drawText(window, uiState.font, isPaused ? "PAUSED" : "RUNNING", cx, cy, 13, isPaused ? sf::Color(255, 100, 100) : sf::Color(100, 255, 100));
            cy += 20;
            drawText(window, uiState.font, "[P] Toggle Pause", cx, cy, 11, sf::Color(150, 150, 150));
            cy += 20;

            // Initial Population
            drawText(window, uiState.font, "INITIAL POPULATION", cx, cy, 14, sf::Color(200, 200, 255));
            cy += 20;
            drawText(window, uiState.font, "Flora: " + std::to_string(uiState.params.initialFlora) + "  [1/-] to change", cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Herbivore: " + std::to_string(uiState.params.initialHerbivore) + "  [2/-] to change", cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Predator: " + std::to_string(uiState.params.initialPredator) + "  [3/-] to change", cx, cy, 13, sf::Color::White);
            cy += 25;

            // Metabolism
            drawText(window, uiState.font, "METABOLISM & ENERGY", cx, cy, 14, sf::Color(200, 200, 255));
            cy += 20;
            drawText(window, uiState.font, "Flora Photosynthesis: " + std::to_string(uiState.params.floraPhotosynthesis).substr(0, 4), cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Herbivore Metabolism: " + std::to_string(uiState.params.herbivoreMetabolism).substr(0, 5), cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Predator Metabolism: " + std::to_string(uiState.params.predatorMetabolism).substr(0, 5), cx, cy, 13, sf::Color::White);
            cy += 25;

            // Vision Ranges
            drawText(window, uiState.font, "VISION RANGES", cx, cy, 14, sf::Color(200, 200, 255));
            cy += 20;
            drawText(window, uiState.font, "Herbivore Food: " + std::to_string(static_cast<int>(uiState.params.herbivoreFoodVision)), cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Herbivore Predator: " + std::to_string(static_cast<int>(uiState.params.herbivorePredatorVision)), cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Herbivore Mate: " + std::to_string(static_cast<int>(uiState.params.herbivoreMateVision)), cx, cy, 13, sf::Color::White);
            cy += 18;
            drawText(window, uiState.font, "Predator Vision: " + std::to_string(static_cast<int>(uiState.params.predatorVision)), cx, cy, 13, sf::Color::White);

            cy += 15;
            drawText(window, uiState.font, "Keys: P=Pause, +=/-=Time, L=Log, R=Reset", cx, cy, 11, sf::Color(150, 150, 150));

            // ============================================================
            // RIGHT TOP: Population Stats
            // ============================================================
            drawPanel(window, 740.f, 40.f, 270.f, 200.f, sf::Color(40, 40, 50, 230));

            cx = 750.f, cy = 50.f;
            drawText(window, uiState.font, "POPULATION STATS", cx, cy, 16, sf::Color(200, 200, 255));
            cy += 25;

            int flora = 0, herb = 0, pred = 0;
            float totalEnergy = 0.f, avgEnergyFlora = 0.f, avgEnergyHerb = 0.f, avgEnergyPred = 0.f;
            int countFlora = 0, countHerb = 0, countPred = 0;

            for (const auto& e : entities) {
                if (e->getType() == Entity::OrganismType::Flora) {
                    flora++;
                    avgEnergyFlora += e->getEnergy();
                    countFlora++;
                } else if (e->getType() == Entity::OrganismType::Herbivore) {
                    herb++;
                    avgEnergyHerb += e->getEnergy();
                    countHerb++;
                } else if (e->getType() == Entity::OrganismType::Predator) {
                    pred++;
                    avgEnergyPred += e->getEnergy();
                    countPred++;
                }
                totalEnergy += e->getEnergy();
            }

            drawText(window, uiState.font, "Flora:        " + std::to_string(flora), cx, cy, 14, sf::Color(100, 255, 100));
            cy += 20;
            drawText(window, uiState.font, "Herbivore: " + std::to_string(herb), cx, cy, 14, sf::Color(255, 215, 0));
            cy += 20;
            drawText(window, uiState.font, "Predator:   " + std::to_string(pred), cx, cy, 14, sf::Color(255, 100, 100));
            cy += 20;
            drawText(window, uiState.font, "Total:       " + std::to_string(entities.size()), cx, cy, 14, sf::Color::White);

            cy += 15;
            drawText(window, uiState.font, "-------------------", cx, cy, 12, sf::Color(100, 100, 100));
            cy += 15;

            if (countFlora > 0) drawText(window, uiState.font, "Avg Flora Energy: " + std::to_string(static_cast<int>(avgEnergyFlora / countFlora)), cx, cy, 12, sf::Color::White);
            cy += 16;
            if (countHerb > 0) drawText(window, uiState.font, "Avg Herb Energy: " + std::to_string(static_cast<int>(avgEnergyHerb / countHerb)), cx, cy, 12, sf::Color::White);
            cy += 16;
            if (countPred > 0) drawText(window, uiState.font, "Avg Pred Energy: " + std::to_string(static_cast<int>(avgEnergyPred / countPred)), cx, cy, 12, sf::Color::White);
            cy += 16;
            drawText(window, uiState.font, "Total Energy: " + std::to_string(static_cast<int>(totalEnergy)), cx, cy, 12, sf::Color::White);

            // ============================================================
            // RIGHT MIDDLE: Experiments Panel
            // ============================================================
            drawPanel(window, 740.f, 250.f, 270.f, 300.f, sf::Color(40, 40, 50, 230));

            cx = 750.f, cy = 260.f;
            drawText(window, uiState.font, "EXPERIMENTS", cx, cy, 16, sf::Color(200, 200, 255));
            cy += 25;

            drawText(window, uiState.font, "ADD ENTITIES:", cx, cy, 13, sf::Color(200, 200, 255));
            cy += 20;

// Add 50 Flora
            drawButton(window, uiState.font, "Add 50 Flora", cx, cy, 200.f, 25.f);
            cy += 30;

            // Add 20 Herbivore
            drawButton(window, uiState.font, "Add 20 Herbivore", cx, cy, 200.f, 25.f);
            cy += 30;

            // Add 5 Predator
            drawButton(window, uiState.font, "Add 5 Predator", cx, cy, 200.f, 25.f);
            cy += 30;

            cy += 10;
            drawText(window, uiState.font, "KILL ENTITIES:", cx, cy, 13, sf::Color(255, 100, 100));
            cy += 20;

            drawButton(window, uiState.font, "Kill 50% Predators", cx, cy, 200.f, 25.f);
            cy += 30;
            drawButton(window, uiState.font, "Kill 50% Herbivores", cx, cy, 200.f, 25.f);
            cy += 30;
            drawButton(window, uiState.font, "Kill All Flora", cx, cy, 200.f, 25.f);
            cy += 30;

            cy += 10;
            drawButton(window, uiState.font, "Reset Simulation", cx, cy, 200.f, 25.f);
            cy += 30;

            cy += 10;
            drawText(window, uiState.font, "DATA EXPORT:", cx, cy, 13, sf::Color(200, 200, 255));
            cy += 20;

            drawButton(window, uiState.font, "Export State (CSV)", cx, cy, 200.f, 25.f);
            cy += 30;

            // Logging checkbox
            std::string loggingStatusText = "Enable CSV Logging: " + std::string(uiState.isLogging ? "ON" : "OFF");
            drawText(window, uiState.font, loggingStatusText, cx, cy, 12, uiState.isLogging ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));

            // ============================================================
            // LEFT BOTTOM: Graphs Panel
            // ============================================================
            drawPanel(window, 10.f, 570.f, 1000.f, 180.f, sf::Color(20, 20, 30, 240));

            drawText(window, uiState.font, "POPULATION HISTORY", 20.f, 580.f, 16, sf::Color(200, 200, 255));

            if (!uiState.floraHistory.empty()) {
                float graphX = 20.f;
                float graphY = 605.f;
                float graphW = 980.f;
                float graphH = 40.f;
                float spacing = 5.f;

                // Flora graph (green)
                drawPanel(window, graphX, graphY, 980.f, graphH, sf::Color(0, 50, 0, 200));
                drawGraphLine(window, uiState.floraHistory, graphX, graphY, 980.f, graphH, sf::Color(100, 255, 100));
                drawText(window, uiState.font, "Flora (Green)", graphX + 5.f, graphY + 2.f, 11, sf::Color(100, 255, 100));

                // Herbivore graph (gold)
                drawPanel(window, graphX, graphY + graphH + 5.f, 980.f, graphH, sf::Color(50, 50, 0, 200));
                drawGraphLine(window, uiState.herbivoreHistory, graphX, graphY + graphH + 5.f, 980.f, graphH, sf::Color(255, 215, 0));
                drawText(window, uiState.font, "Herbivore (Gold)", graphX + 5.f, graphY + graphH + 7.f, 11, sf::Color(255, 215, 0));

                // Predator graph (red)
                drawPanel(window, graphX, graphY + 2 * (graphH + 5.f), 980.f, graphH, sf::Color(50, 0, 0, 200));
                drawGraphLine(window, uiState.predatorHistory, graphX, graphY + 2 * (graphH + 5.f), 980.f, graphH, sf::Color(255, 100, 100));
                drawText(window, uiState.font, "Predator (Red)", graphX + 5.f, graphY + 2 * (graphH + 5.f) + 2.f, 11, sf::Color(255, 100, 100));
            } else {
                drawText(window, uiState.font, "No data yet...", 20.f, 610.f, 14, sf::Color(150, 150, 150));
            }

            // ============================================================
            // BOTTOM: Logging Panel
            // ============================================================
            drawPanel(window, 10.f, 760.f, 1000.f, 40.f, sf::Color(20, 20, 30, 240));
            drawText(window, uiState.font, "CSV Logging: " + std::string(uiState.isLogging ? "ENABLED" : "DISABLED") + "  [L] Toggle  |  [E] Export State", 20.f, 770.f, 12, uiState.isLogging ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));

            // ============================================================
            // TOP: Menu Bar (drawn last so it's on top of everything)
            // ============================================================
            sf::RectangleShape menuBar(sf::Vector2f(1024.f, 25.f));
            menuBar.setFillColor(sf::Color(40, 40, 50));
            window.draw(menuBar);

            sf::Text titleText(uiState.font, "Artificial Life - FSM Ecosystem", 14);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition(sf::Vector2f(10.f, 4.f));
            window.draw(titleText);

            sf::Text pauseText(uiState.font, isPaused ? "[P] PAUSED" : "[P] Running", 12);
            pauseText.setFillColor(isPaused ? sf::Color(255, 100, 100) : sf::Color(100, 255, 100));
            pauseText.setPosition(sf::Vector2f(200.f, 4.f));
            window.draw(pauseText);

            sf::Text timeText(uiState.font, "[+/-] Time: " + std::to_string(uiState.params.timeScale).substr(0, 4) + "x", 12);
            timeText.setFillColor(sf::Color(200, 200, 200));
            timeText.setPosition(sf::Vector2f(320.f, 4.f));
            window.draw(timeText);

            sf::Text logText(uiState.font, "[L] Logging: " + std::string(uiState.isLogging ? "ON" : "OFF"), 12);
            logText.setFillColor(uiState.isLogging ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
            logText.setPosition(sf::Vector2f(500.f, 4.f));
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