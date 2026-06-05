#pragma once

#include "map.hpp"
#include "tile_registry.hpp"
#include <vector>

struct PlacedDecoration {
    std::string spriteKey;
    int gridX;
    int gridY;
};

class WorldGenerator {
private:
    void CarveRiverAgent(Map& map, int startX, int startY, TileRuntimeId riverId, const std::vector<double>& elevationGrid);
    void GenerateCrossings(Map& map, int spacingFrequency);
    void GenerateCliffs(Map& map);
    void DistributeVegetation(Map& map);
    
    std::vector<PlacedDecoration> m_decorations;

public:
    WorldGenerator() = default;
        ~WorldGenerator() = default;


    void Generate(Map& map, unsigned int seed);

    const std::vector<PlacedDecoration>& GetDecorations() const noexcept { return m_decorations;}
};