#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class TileType : char;

class Chunk {
private:
    int m_chunkX;
    int m_chunkY;
    int m_chunkSize;
    int m_tileSize;

    std::vector<TileType> m_localGrid;

    sf::VertexArray m_floorVerticies;
    sf::VertexArray m_wallVerticies;

public:
    Chunk(int chunkX, int chunkY, int chunkSize, int tileSize);

    const sf::VertexArray& GetFloorVerticies() const { return m_floorVerticies; }
    const sf::VertexArray& GetWallVerticies() const { return m_wallVerticies; }

    TileType GetTile(int localX, int localY) const;
    void SetTile(int localX, int localY, TileType type);

    void UpdateGeometry(int mapWidth, int mapHeight);
};