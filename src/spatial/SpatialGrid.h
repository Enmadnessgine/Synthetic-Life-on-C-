#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cmath>
#include "../entities/Entity.h"

class SpatialGrid {
public:
    // Cell size in pixels - determines grid resolution
    static constexpr float CELL_SIZE = 100.f;

    SpatialGrid(float worldWidth, float worldHeight, float cellSize = CELL_SIZE);
    ~SpatialGrid() = default;

    // Add an entity to the grid based on its position
    void addEntity(const sf::Vector2f& position, int entityIndex);

    // Remove an entity from the grid
    void removeEntity(const sf::Vector2f& position, int entityIndex);

    // Update entity position in grid
    void updateEntity(const sf::Vector2f& oldPos, const sf::Vector2f& newPos, int entityIndex);

    // Get all entities that could interact with given position (broadphase)
    std::vector<int> getNearbyEntities(const sf::Vector2f& position) const;

    // Clear the grid
    void clear();

    // Get nearby entities filtered by type
    std::vector<int> getNearbyEntitiesOfType(const sf::Vector2f& position, float radius, Entity::OrganismType targetType, const std::vector<std::unique_ptr<Entity>>& entities) const;

    // Get number of cells in grid
    int getGridWidth() const { return gridWidth_; }
    int getGridHeight() const { return gridHeight_; }

private:
    int gridWidth_;
    int gridHeight_;
    int cellSize_;
    // 2D array of entity indices per cell
    // Using flat 1D vector: cellIndex = y * gridWidth_ + x
    std::vector<std::vector<int>> grid_;
};