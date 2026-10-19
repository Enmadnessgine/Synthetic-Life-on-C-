#include "SpatialGrid.h"

SpatialGrid::SpatialGrid(float worldWidth, float worldHeight)
    : cellSize_(CELL_SIZE)
{
    // Calculate grid dimensions (ceil to get full coverage)
    gridWidth_ = static_cast<int>(std::ceil(worldWidth / cellSize_));
    gridHeight_ = static_cast<int>(std::ceil(worldHeight / cellSize_));

    // Initialize grid with empty cells
    grid_.resize(gridHeight_ * gridWidth_);
}

void SpatialGrid::addEntity(const sf::Vector2f& position, int entityIndex) {
    // Determine which cell this entity belongs to
    int cellX = static_cast<int>(std::floor(position.x / cellSize_));
    int cellY = static_cast<int>(std::floor(position.y / cellSize_));

    // Clamp to grid bounds
    if (cellX < 0) cellX = 0;
    if (cellX >= gridWidth_) cellX = gridWidth_ - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= gridHeight_) cellY = gridHeight_ - 1;

    int cellIndex = cellY * gridWidth_ + cellX;
    grid_[cellIndex].push_back(entityIndex);
}

void SpatialGrid::removeEntity(const sf::Vector2f& position, int entityIndex) {
    int cellX = static_cast<int>(std::floor(position.x / cellSize_));
    int cellY = static_cast<int>(std::floor(position.y / cellSize_));

    if (cellX < 0 || cellX >= gridWidth_ || cellY < 0 || cellY >= gridHeight_) {
        return;
    }

    int cellIndex = cellY * gridWidth_ + cellX;
    auto& cell = grid_[cellIndex];

    // Find and remove the entity index from this cell
    for (auto it = cell.begin(); it != cell.end(); ++it) {
        if (*it == entityIndex) {
            cell.erase(it);
            return;
        }
    }
}

void SpatialGrid::updateEntity(const sf::Vector2f& oldPos, const sf::Vector2f& newPos, int entityIndex) {
    // Remove from old position cell
    removeEntity(oldPos, entityIndex);

    // Add to new position cell
    addEntity(newPos, entityIndex);
}

std::vector<int> SpatialGrid::getNearbyEntities(const sf::Vector2f& position) const {
    std::vector<int> neighbors;

    // Determine which cells to check (3x3 area around the position)
    int cellX = static_cast<int>(std::floor(position.x / cellSize_));
    int cellY = static_cast<int>(std::floor(position.y / cellSize_));

    // Clamp to grid bounds
    if (cellX < 0) cellX = 0;
    if (cellX >= gridWidth_) cellX = gridWidth_ - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= gridHeight_) cellY = gridHeight_ - 1;

    // Check 3x3 neighborhood of cells
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int checkX = cellX + dx;
            int checkY = cellY + dy;

            if (checkX >= 0 && checkX < gridWidth_ && checkY >= 0 && checkY < gridHeight_) {
                int cellIndex = checkY * gridWidth_ + checkX;
                const auto& cell = grid_[cellIndex];

                // Add all entity indices in this cell (skip the entity at position itself)
                for (int idx : cell) {
                    neighbors.push_back(idx);
                }
            }
        }
    }

    return neighbors;
}