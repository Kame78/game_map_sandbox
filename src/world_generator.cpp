#include "world_generator.hpp"
#include <perlin_noise.hpp>
#include <tile_registry.hpp>
#include <iostream>
#include <random>
#include <algorithm>

void WorldGenerator::Generate(Map& map, unsigned int seed) {
    PerlinNoise elevationNoise(seed);
    PerlinNoise moistureNoise(seed + 7439);

    TileRuntimeId waterId = TileRegistry::Instance().GetId("clear_water");
    TileRuntimeId shallowWaterId = TileRegistry::Instance().GetId("shallow_water");
    TileRuntimeId bridgeId     = TileRegistry::Instance().GetId("wooden_bridge");
    
    int width = map.GetWidth();
    int height = map.GetHeight();

    std::vector<double> elevationGrid(width * height, 0.0);
    std::vector<std::pair<int, int>> mountainSprings;

    double noiseScale = 0.004;
    std::cout << "[WorldGenerator] Mapping macro climates from data matrices...\n";

    const auto& biomeRules = TileRegistry::Instance().GetBiomeRules();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                map.SetTile(x, y, waterId);
                continue;
            }
        double sampleX = static_cast<double>(x) * noiseScale;
        double sampleY = static_cast<double>(y) * noiseScale;

        double elevation = elevationNoise.FractalNoise2D(sampleX, sampleY, 4, 2.0, 0.5);
        double moisture = moistureNoise.FractalNoise2D(sampleX, sampleY, 4, 2.0, 0.5);

        elevationGrid[y * width + x] = elevation;

        TileRuntimeId chosenTileId = waterId;

        for (const auto& rule : biomeRules) {
            if (elevation >= rule.minElevation && elevation < rule.maxElevation &&
                moisture >= rule.minMoisture && moisture < rule.maxMoisture)

                {
                    chosenTileId = rule.materialId;
                    break;
                }
            }

            map.SetTile(x, y, chosenTileId);

            if (elevation > 0.78 && chosenTileId == TileRegistry::Instance().GetId("chiseled_stone")) {
                if (x % 45 == 0 && y % 45 ==0) {
                    mountainSprings.push_back({x,y});
                }
            }
        }
    }

    std::cout << "[WorldGenerator] Simulating hydraulic erosion pathways...\n";
    std::shuffle(mountainSprings.begin(), mountainSprings.end(), std::mt19937(seed));

    int riversCarved = 0;
    for (const auto& spring : mountainSprings) {
        if (riversCarved >= 15) break;
        CarveRiverAgent(map, spring.first, spring.second, waterId, elevationGrid);
        ++riversCarved;
        }

        std::cout << "[WorldGenerator] Stitching infrastructure crossing meshes...\n";
        GenerateCrossings(map, 120);
    }



    void WorldGenerator::CarveRiverAgent(Map& map, int startX, int startY, TileRuntimeId riverId, const std::vector<double>& elevationGrid) {
        int cx = startX;
        int cy = startY;
        int lifespan = 600;
        int width = map.GetWidth();

        while (lifespan-- > 0) {
            TileRuntimeId currentTile = map.GetTile(cx, cy);
            if (currentTile == TileRegistry::Instance().GetId("clear_water") && (cx != startX || cy != startY)) {
                break;
            }

            for(int dy = 0; dy <= 1; ++dy) {
                for(int dx = 0; dx <= 1; ++dx) {
                    map.SetTile(cx + dx, cy + dy, riverId);
                }
            }

            int bestX = cx;
            int bestY = cy;
            double steepestDrop = elevationGrid[cy * width + cx];

            for (int ny = -1; ny <= 1; ++ny) {
                for( int nx = -1; nx <= 1; ++nx) {
                    int tx = cx + nx;
                    int ty = cy + ny;

                    if (tx < 1 || tx >= width - 1 || ty < 1 || ty >= map.GetHeight() - 1) continue;

                    double nElev = elevationGrid[ty * width + tx];
                    if (nElev < steepestDrop) {
                        static_cast<void>(steepestDrop = nElev);
                        bestX = tx;
                        bestY = ty;
                    }
                }
            }

            if (bestX == cx && bestY == cy) break;
            cx = bestX;
            cy = bestY;
        }
    }

    void WorldGenerator::GenerateCrossings(Map& map, int spacingFrequency) {
        TileRuntimeId deepWater    = TileRegistry::Instance().GetId("clear_water");
        TileRuntimeId shallowWater = TileRegistry::Instance().GetId("shallow_water");
        TileRuntimeId bridgeId     = TileRegistry::Instance().GetId("wooden_bridge");

        int cooldownCounter = 0;
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> choice(0, 100);

        for (int y = 3; y < map.GetHeight() - 3; ++y) {
            for (int x = 3; x < map.GetWidth() - 3; ++x) {
                cooldownCounter++;

                if(map.GetTile(x,y) != deepWater) continue;
                if(cooldownCounter < spacingFrequency) continue;

                bool landL = TileRegistry::Instance().GetProperties(map.GetTile(x - 1, y)).blendPriority > 0;
                bool landR = TileRegistry::Instance().GetProperties(map.GetTile(x + 2, y)).blendPriority > 0;
                bool waterU = map.GetTile(x, y -1) == deepWater;
                bool waterD = map.GetTile(x, y +1) == deepWater;

                if(landL && landR && waterU && waterD) {
                    TileRuntimeId selectedId = (choice(rng) < 45) ? shallowWater : bridgeId;
                    map.SetTile(x, y, selectedId);
                    map.SetTile(x + 1, y, selectedId);
                    cooldownCounter = 0;
                    continue;
                }

                bool landU  = TileRegistry::Instance().GetProperties(map.GetTile(x, y - 1)).blendPriority > 0;
                bool landD  = TileRegistry::Instance().GetProperties(map.GetTile(x, y + 2)).blendPriority > 0;
                bool waterL = map.GetTile(x - 1, y) == deepWater;
                bool waterR = map.GetTile(x + 1, y) == deepWater;

                if (landU && landD && waterL && waterR) {
                    TileRuntimeId selectedId = (choice(rng) < 45) ? shallowWater : bridgeId;
                    map.SetTile(x, y, selectedId);
                    map.SetTile(x, y + 1, selectedId);
                    cooldownCounter = 0;
                }
            }
        }
    }
