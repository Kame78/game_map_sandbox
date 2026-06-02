#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class TileType : char {
    Floor = '.',
    Wall = '#'
};

class Map : public sf::Drawable {
private:
    int m_width{0};
    int m_height{0};
    std::vector<TileType> m_grid;

    const int TILE_SIZE = 32;

    sf::VertexArray m_floorVertices;
    sf::VertexArray m_wallVertices;

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
    void UpdateGeometry();

    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType type);

protected:
    virtual void draw(sf::RenderTarget& target, const sf::RenderStates states) const override;
};