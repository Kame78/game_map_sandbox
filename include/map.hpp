#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "chunk.hpp"

enum class TileType : char {
    Floor = '.',
    Wall = '#'
};

class Map : public sf::Drawable {
private:
    int m_width{0};
    int m_height{0};
    
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int TILE_SIZE = 32;

    int m_numChunksX{0};
    int m_numChunksY{0};

    std::vector<Chunk> m_chunks;

    sf::Texture m_grassTexture;
    sf::Texture m_wallTexture;

public:
    Map(int width, int height);

    void Randomize(int fillPercentage);
    void StepSimulation();
    int CountActiveNeighbors(int x, int y) const;

    int GetWidth() const{ return m_width; }
    int GetHeight() const{ return m_height; }

    void PrintToConsole() const;

    void SaveToFile(const std::string& filepath, int playerX, int playerY) const;
    void LoadFromFile(const std::string& filepath, int& outPlayerX, int& outPlayerY);

    bool LoadTextures(const std::string& grassPath, const std::string& wallPath);

    void UpdateAllGeometry();

    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType type, bool updateGeometry = true);

protected:
    virtual void draw(sf::RenderTarget& target, const sf::RenderStates states) const override;
};