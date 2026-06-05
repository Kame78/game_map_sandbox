#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include "map.hpp"
#include "world_generator.hpp"

struct AtlasRect {
    int x, y, w, h;
};

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

    sf::Texture m_vegetationAtlasTexture;
    std::unordered_map<std::string, AtlasRect> m_spriteAtlas;

public:

    MapRenderer() = default;

    bool Initialize();
    bool LoadSpriteAtlas(const std::string& configPath);
    void RegenarateAllGeometry(const Map& map);
    void Draw(sf::RenderTarget& target, const sf::RenderStates states) const;
    bool ConnectsTo(TileRuntimeId currentId, TileRuntimeId neighborId);
    void DrawDecorations(sf::RenderTarget& target, const std::vector<PlacedDecoration>& decorations, const Map& map) const;
};