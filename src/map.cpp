#include "map.hpp"
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
       
bool Map::LoadTextures(const std::string &grassPath, const std::string& wallPath) {
    if (!m_grassTexture.loadFromFile(grassPath) || !m_wallTexture.loadFromFile(wallPath)) {
        return false;
    }
    return true;
}

void Map::UpdateAllGeometry() {
    for (auto& chunk : m_chunks) {
        chunk.UpdateGeometry(m_width, m_height);
    }
}

void Map::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
    sf::RenderStates floorStates = states;
    floorStates.texture = &m_grassTexture;
    for (const auto& chunk : m_chunks) {
        target.draw(chunk.GetFloorVerticies(), floorStates);
    }

    sf::RenderStates wallStates = states;
    wallStates.texture = &m_wallTexture;
    for (const auto& chunk : m_chunks) {
        target.draw(chunk.GetWallVerticies(), wallStates);
    }
}

 TileType Map::getTile(int x, int y) const {
    if ( x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return TileType::Wall;
    }

    int cx = x / CHUNK_SIZE;
    int cy = y / CHUNK_SIZE;
    int chunkIndex = cy * m_numChunksX + cx;

    int lx = x % CHUNK_SIZE;
    int ly = y % CHUNK_SIZE;

    return m_chunks[chunkIndex].GetTile(lx, ly);
 }

 void Map::setTile(int x, int y, TileType type, bool updateGeometry) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;

    int cx = x / CHUNK_SIZE;
    int cy = y / CHUNK_SIZE;
    int chunkIndex = cy * m_numChunksX + cx;

    int lx = x % CHUNK_SIZE;
    int ly = y % CHUNK_SIZE;

    m_chunks[chunkIndex].SetTile(lx, ly, type);

    if (updateGeometry) {
        m_chunks[chunkIndex].UpdateGeometry(m_width, m_height);
    }
 }

 void Map::PrintToConsole() const {
    for (int y = 0; y < m_height; ++y) {
        std::string rowString ="";
        for (int x = 0; x < m_width; ++x) {
            rowString += static_cast<char>(getTile(x, y));
        }
        std::cout << rowString << "\n";
    }
 }

 void Map::Randomize(int fillPercentage) {
    std::random_device rd;
    std::mt19937 prng(rd());
    std::uniform_int_distribution<int> dist(0, 100);

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if(x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1 ) {
                setTile(x, y, TileType::Wall);
            } else{
                TileType type = (dist(prng) < fillPercentage) ? TileType::Wall : TileType::Floor;
                setTile(x, y, type, false);
            }
        }
    }
    UpdateAllGeometry();
}

int Map::CountActiveNeighbors(int x, int y) const {
    int wallCount = 0;

    for (int neighborY = y -1; neighborY <= y + 1; ++neighborY) {
        for (int neighborX = x - 1; neighborX <= x + 1; ++neighborX) {
            if (neighborX == x && neighborY == y) {
                continue;
            }
            if (getTile(neighborX, neighborY) == TileType::Wall) {
                ++wallCount;
            }
        }
    }
    return wallCount;
}

void Map::StepSimulation() {
    std::vector<TileType> nextGrid(m_width * m_height, TileType::Floor);

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if ( x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1) {
                nextGrid[y * m_width + x] = TileType::Wall;
                continue;
            }
            int neighbors = CountActiveNeighbors(x, y);
            
            if (getTile(x, y) == TileType::Wall) {
                nextGrid[y * m_width + x] = (neighbors >= 4) ? TileType::Wall : TileType::Floor;
            } else {
                nextGrid[y * m_width + x] = (neighbors >= 5) ? TileType::Wall : TileType::Floor;
            }
        }
    }
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            setTile(x, y, nextGrid[y * m_width + x], false);
        }
    }
    UpdateAllGeometry();
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
            TileType type = getTile(x, y);
            out.write(reinterpret_cast<const char*>(&type), sizeof(TileType));
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
            TileType type;
            in.read(reinterpret_cast<char*>(&type), sizeof(TileType));
            setTile(x, y, type, false);
        }
    }
    
    in.read(reinterpret_cast<char*>(&outPlayerX), sizeof(outPlayerX));
    in.read(reinterpret_cast<char*>(&outPlayerY), sizeof(outPlayerY));

    in.close();

    UpdateAllGeometry();
    std::cout << "Successfully loaded map configuration from " << filepath << "\n";
}

