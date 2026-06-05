#pragma once

#include "map.hpp"
#include "tile_registry.hpp"
#include <vector>

class WorldGenerator {
private:
    void CarveRiverAgent(Map& map, int startX, int startY, TileRuntimeId riverId, const std::vector<double>& elevationGrid);
    void GenerateCrossings(Map& map, int spacingFrequency);

public:
    WorldGenerator() = default;
        ~WorldGenerator() = default;


    void Generate(Map& map, unsigned int seed);
};