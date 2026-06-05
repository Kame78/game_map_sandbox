#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include "map.hpp"

class MapRenderer {
private:
    struct RenderChunk {
        std::unordered_map<std::string, sf::VertexArray> baseLayers;
        std::unordered_map<std::string, sf::VertexArray> alphaLayers;
};

    std::unordered_map<std::string, sf::Texture> m_textureCache;
    
    int m_numChunksX{};
    int m_numChunksY{};
    std::vector<RenderChunk> m_renderChunks;

    void RebuildChunkGeometry(int cx, int cy, const Map& map);

public:

    MapRenderer() = default;

    bool Initialize();
    void RegenarateAllGeometry(const Map& map);
    void Draw(sf::RenderTarget& target, const sf::RenderStates states) const;
    bool ConnectsTo(TileRuntimeId currentId, TileRuntimeId neighborId);
};