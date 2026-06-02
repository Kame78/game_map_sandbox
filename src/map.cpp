#include "map.hpp"
#include <iostream>
#include <stdexcept>
#include <random>
#include <fstream>

Map::Map(int width, int height)
    : m_width(width), m_height(height), m_grid(width * height, TileType::Floor) {
        m_floorVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_wallVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    }

bool Map::LoadTextures(const std::string& grassPath, const std::string& wallPath) {
    if (!m_grassTexture.loadFromFile(grassPath) || !m_wallTexture.loadFromFile(wallPath)) {
        return false;
    }
    return true;
}

void Map::UpdateGeometry() {
    m_floorVertices.clear();
    m_wallVertices.clear();

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            TileType type = getTile(x, y);

            float screenX1 = static_cast<float>(x * TILE_SIZE);
            float screenY1 = static_cast<float>(y * TILE_SIZE);
            float screenX2 = screenX1 + TILE_SIZE;
            float screenY2 = screenY1 + TILE_SIZE;

            float texU1 = 0.f;
            float texV1 = 0.f;

            if (type == TileType::Wall) {
                texU1 = 32.f;
                texV1 = 288.f;
            } else {
                int variation = (x * 7 + y * 13) % 3;
                texU1 = static_cast<float>(variation * TILE_SIZE);
                texV1 = 0.f;
            }

            float texU2 = texU1 + TILE_SIZE;
            float texV2 = texV1 + TILE_SIZE;

            sf::Vertex topLeft, topRight, bottomLeft, bottomRight;
            topLeft.position = {screenX1, screenY1};
            topRight.position = {screenX2, screenY1};
            bottomRight.position = {screenX2, screenY2};
            bottomLeft.position = {screenX1, screenY2};

            topLeft.texCoords = {texU1, texV1};
            topRight.texCoords = {texU2, texV1};
            bottomRight.texCoords = {texU2, texV2};
            bottomLeft.texCoords = {texU1, texV2};

            sf::VertexArray& targetLayer = (type == TileType::Wall) ? m_wallVertices : m_floorVertices;

            targetLayer.append(topLeft);
            targetLayer.append(topRight);
            targetLayer.append(bottomRight);

            targetLayer.append(topLeft);
            targetLayer.append(bottomRight);
            targetLayer.append(bottomLeft);
        }
    }
}

void Map::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
    sf::RenderStates floorStates = states;
    floorStates.texture = &m_grassTexture;
    target.draw(m_floorVertices, floorStates);

    sf::RenderStates wallStates = states;
    wallStates.texture = &m_wallTexture;
    target.draw(m_wallVertices, wallStates);

}

 TileType Map::getTile(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return TileType::Wall;
    }
    
    return m_grid[y * m_width + x];
 }

 void Map::setTile(int x, int y, TileType type) {
    if (x >= 0 && x < m_width && y>=0 && y<m_height) {
        m_grid[y * m_width + x] = type;
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
                setTile(x, y, type);
            }
        }
    }
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
    m_grid = nextGrid;
}

void Map::SaveToFile(const std::string& filepath, int playerX, int playerY) const {
    std::ofstream out(filepath, std::ios::binary);

    if(!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << "\n";
        return;
    }

    out.write(reinterpret_cast<const char*>(&m_width), sizeof(m_width));
    out.write(reinterpret_cast<const char*>(&m_height), sizeof(m_height));

    out.write(reinterpret_cast<const char*>(m_grid.data()), m_grid.size() * sizeof(TileType));

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
    m_grid.resize(m_width * m_height);

    in.read(reinterpret_cast<char*>(m_grid.data()), m_grid.size() * sizeof(TileType));

    in.read(reinterpret_cast<char*>(&outPlayerX), sizeof(outPlayerX));
    in.read(reinterpret_cast<char*>(&outPlayerY), sizeof(outPlayerY));
    
    in.close();

    UpdateGeometry();
    std::cout << "Successfully loaded map configuration from " << filepath << "\n";
}

