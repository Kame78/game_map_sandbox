#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include "map.hpp"

class MapRenderer {
private:
    struct RenderChunk {
        sf::VertexArray baseLayerVerticies;
        sf::VertexArray aplhaLayerVerticies;
};

    sf::Texture m_masterTilesetTexture;

    int m_numChunksX{};
    int m_numChunksY{};
    std::vector<RenderChunk> m_renderChunks;

    void RebuildChunkGeometry(int cx, int cy, const Map& map);

    void BuildSubTileQuad(sf::VertexArray& va, float screenX, float screenY, int subX, int subY, sf::Vector2i atlasSource);
    void BuildFullTile(sf::VertexArray& va, float screenX, float screenY, int bitmaskValue, sf::Vector2i atlasSource);

public:

    MapRenderer() = default;

    bool LoadTextures(const std::string& combinedTexturePath);

    bool Initialize(const std::string& configJsonPath);

    void RegenarateAllGeometry(const Map& map);

    void Draw(sf::RenderTarget& target, const sf::RenderStates states) const;
};