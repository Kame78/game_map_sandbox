#include "map.hpp"
#include "tile_registry.hpp"
#include <iostream>
#include <stdexcept>
#include <random>
#include <fstream>

Map::Map(int width, int height)
    : m_width(width), m_height(height)
{
    m_numChunksX = (width + CHUNK_SIZE -1) / CHUNK_SIZE;
    m_numChunksY = (height + CHUNK_SIZE -1) / CHUNK_SIZE;

    m_chunks.reserve(m_numChunksX * m_numChunksY);

    for (int cy = 0; cy < m_numChunksY; ++cy) {
        for(int cx = 0; cx< m_numChunksX; ++cx) {
            m_chunks.emplace_back(cx, cy, CHUNK_SIZE, TILE_SIZE);
        }
    }
}
       
 TileRuntimeId Map::GetTile(int x, int y) const {
    if ( x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return TileRegistry::Instance().GetId("chiseled_stone_dark_edges");
    }

    int cx = x / CHUNK_SIZE;
    int cy = y / CHUNK_SIZE;
    int chunkIndex = cy * m_numChunksX + cx;

    return m_chunks[chunkIndex].GetTile(x % CHUNK_SIZE, y % CHUNK_SIZE);
 }

 void Map::SetTile(int x, int y, TileRuntimeId typeId) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;

    int cx = x / CHUNK_SIZE;
    int cy = y / CHUNK_SIZE;
    

    m_chunks[cy * m_numChunksX + cx].SetTile(x % CHUNK_SIZE, y % CHUNK_SIZE, typeId);
 }
 
 void Map::Randomize(int fillPercentage) {
    std::random_device rd;
    std::mt19937 prng(rd());
    std::uniform_int_distribution<int> dist(0, 100);

    TileRuntimeId wallId = TileRegistry::Instance().GetId("chiseled_stone_dark_edges");
    TileRuntimeId floorId = TileRegistry::Instance().GetId("medium_grass_standard");

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if(x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1 ) {
                SetTile(x, y, wallId);
            } else{
                TileRuntimeId chosenId = (dist(prng) < fillPercentage) ? wallId : floorId;
                SetTile(x, y, chosenId);
            }
        }
    }
}

int Map::CountActiveNeighbors(int x, int y) const {
    int wallCount = 0;

    for (int neighborY = y -1; neighborY <= y + 1; ++neighborY) {
        for (int neighborX = x - 1; neighborX <= x + 1; ++neighborX) {
            if (neighborX == x && neighborY == y) continue;

            TileRuntimeId id = GetTile(neighborX, neighborY);
            if (TileRegistry::Instance().GetProperties(id).isSolid) {
                ++wallCount;
            }
        }
    }
    return wallCount;
}

void Map::StepSimulation() {
    std::vector<TileRuntimeId> nextGrid(m_width * m_height, 0);
    TileRuntimeId wallId = TileRegistry::Instance().GetId("chiseled_stone_dark_edges");
    TileRuntimeId floorId = TileRegistry::Instance().GetId("medium_grass_standard");


    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if ( x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1) {
                nextGrid[y * m_width + x] = wallId;
                continue;
            }
            int neighbors = CountActiveNeighbors(x, y);
            bool standsAsSolid = (TileRegistry::Instance().GetProperties(GetTile(x, y)).isSolid);
            
            if (standsAsSolid) {
                nextGrid[y * m_width + x] = (neighbors >= 4) ? wallId : floorId;
            } else {
                nextGrid[y * m_width + x] = (neighbors >= 5) ? wallId : floorId;
            }
        }
    }
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            SetTile(x, y, nextGrid[y * m_width + x]);
        }
    }
}

void Map::SaveToFile(const std::string& filepath, int playerX, int playerY) const {
    std::ofstream out(filepath, std::ios::binary);

    if(!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << "\n";
        return;
    }

    out.write(reinterpret_cast<const char*>(&m_width), sizeof(m_width));
    out.write(reinterpret_cast<const char*>(&m_height), sizeof(m_height));

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            TileRuntimeId id = GetTile(x, y);
            out.write(reinterpret_cast<const char*>(&id), sizeof(TileRuntimeId));
        }
    }

    out.write(reinterpret_cast<const char*>(&playerX), sizeof(playerX));
    out.write(reinterpret_cast<const char*>(&playerY), sizeof(playerY));


    out.close();
    std::cout << "Successfully saved map configuration to " << filepath << "\n";
}

void Map::LoadFromFile(const std::string& filepath, int& outPlayerX, int& outPlayerY) {
    std::ifstream in(filepath, std::ios::binary);

    if (!in.is_open()) {
        std::cerr << "Failed to open file for reading: " << filepath << "\n";
        return;
    }

    int loadedWidth{};
    int loadedHeight{};
    in.read(reinterpret_cast<char*>(&loadedWidth), sizeof(loadedWidth));
    in.read(reinterpret_cast<char*>(&loadedHeight), sizeof(loadedHeight));

    m_width = loadedWidth;
    m_height = loadedHeight;

    m_numChunksX = (loadedWidth + CHUNK_SIZE -1) / CHUNK_SIZE;
    m_numChunksY = (loadedHeight + CHUNK_SIZE -1) / CHUNK_SIZE;

    m_chunks.clear();
    m_chunks.reserve(m_numChunksX * m_numChunksY);

    for (int cy = 0; cy < m_numChunksY; ++cy) {
        for(int cx = 0; cx< m_numChunksX; ++cx) {
            m_chunks.emplace_back(cx, cy, CHUNK_SIZE, TILE_SIZE);
        }
    }

    for (int y = 0; y < loadedHeight; ++y) {
        for (int x = 0; x < loadedWidth; ++x) {
            TileRuntimeId id;
            in.read(reinterpret_cast<char*>(&id), sizeof(TileRuntimeId));
            SetTile(x, y, id);
        }
    }
    
    in.read(reinterpret_cast<char*>(&outPlayerX), sizeof(outPlayerX));
    in.read(reinterpret_cast<char*>(&outPlayerY), sizeof(outPlayerY));

    in.close();
    std::cout << "Successfully loaded map configuration from " << filepath << "\n";
}

