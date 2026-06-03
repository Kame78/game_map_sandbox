#pragma once

#include <SFML/Graphics.hpp>
#include "map.hpp"

class MapRenderer {
private:
    struct RenderChunk {
        sf::VertexArray floorVerticies;
        sf::VertexArray wallVerticies;
};

    sf::Texture m_grassTexture;
    sf::Texture m_wallTexture;

    int m_numChunksX{};
    int m_numChunksY{};

    std::vector<RenderChunk> m_renderChunks;

    void RebuildChunkGeometry(int cx, int cy, const Map& map);

public:

    MapRenderer() = default;

    bool LoadTextures(const std::string& grassPath, const std::string& wallPath);

    bool LoadTextures(const std::string& grassPath, const std::string& wallPath);

    void RegenarateAllGeometry(const Map& map);

    void Draw(sf::RenderTarget& target, const sf::RenderStates states) const;
};