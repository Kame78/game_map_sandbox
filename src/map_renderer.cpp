#include "map_renderer.hpp"
#include "tile_registry.hpp"
#include <iostream>

bool MapRenderer::LoadTextures(const std::string& combinedTexturePath) {
    if (!m_masterTilesetTexture.loadFromFile(combinedTexturePath)) {
        std::cerr << "Renderer Error: Failed to load atlas file: " << combinedTexturePath << "\n";
        return false;
    }
    return true;
}

void MapRenderer::RegenarateAllGeometry(const Map& map) {
    m_numChunksX = map.GetNumChunksX();
    m_numChunksY = map.GetNumChunksY();

    m_renderChunks.clear();
    m_renderChunks.resize(m_numChunksX * m_numChunksY);

    for (int cy = 0; cy < m_numChunksY; ++cy) {
        for (int cx = 0; cx < m_numChunksX; ++cx) {
            RebuildChunkGeometry(cx, cy, map);
        }
    }
}

void MapRenderer::RebuildChunkGeometry(int cx, int cy, const Map& map) {
    int index = cy * m_numChunksX + cx;

    if (index < 0 || index >= static_cast<int>(m_renderChunks.size())) {
        return;
    }
    RenderChunk& rChunk = m_renderChunks[index];

    rChunk.baseLayerVerticies.setPrimitiveType(sf::PrimitiveType::Triangles);
    rChunk.aplhaLayerVerticies.setPrimitiveType(sf::PrimitiveType::Triangles);

    rChunk.baseLayerVerticies.clear();
    rChunk.aplhaLayerVerticies.clear();

    int chunkSize = map.GetChunkSize();
    int tileSize = map.GetTileSize();
    int chunkStartX = cx * chunkSize;
    int chunkStartY = cy * chunkSize;

    for (int y = 0; y < chunkSize; ++y) {
        for (int x = 0; x < chunkSize; ++x) {
            int globalX = chunkStartX + x;
            int globalY = chunkStartY + y;

            if (globalX >=map.GetWidth() || globalY >= map.GetHeight()) continue;
            
            TileRuntimeId tileId = map.GetTile(globalX, globalY);
            const auto& props = TileRegistry::Instance().GetProperties(tileId);

            float screenX1 = static_cast<float>(globalX * tileSize);
            float screenY1 = static_cast<float>(globalY * tileSize);
            float screenX2 = screenX1 + tileSize;
            float screenY2 = screenY1 + tileSize;

            float texU1 = static_cast<float>(props.startX) + (tileSize * 2);
            float texV1 = static_cast<float>(props.startY) + (tileSize * 3);
            float texU2 = texU1 + tileSize;
            float texV2 = texV1 + tileSize;

            sf::Vertex topLeft, topRight, bottomLeft, bottomRight;
            topLeft.position = { screenX1, screenY1 };
            topRight.position = { screenX2, screenY1 };
            bottomLeft.position = { screenX1, screenY2};
            bottomRight.position = { screenX2, screenY2 };

            topLeft.texCoords = { texU1, texV1 };
            topRight.texCoords = { texU2, texV1 };
            bottomLeft.texCoords = { texU1, texV2 };
            bottomRight.texCoords = { texU2, texV2 };

            sf::VertexArray& targetLayer = (props.isSolid) ? rChunk.baseLayerVerticies : rChunk.aplhaLayerVerticies;

            targetLayer.append(topLeft); targetLayer.append(topRight); targetLayer.append(bottomRight);
            targetLayer.append(topLeft); targetLayer.append(bottomRight); targetLayer.append(bottomLeft);

        }
    }
}

void MapRenderer::Draw(sf::RenderTarget& target, const sf::RenderStates states) const {
    sf::RenderStates activeStates = states;
    activeStates.texture = &m_masterTilesetTexture;

    for (const auto& chunk : m_renderChunks) {
        target.draw(chunk.baseLayerVerticies, activeStates);
}

    for (const auto& chunk : m_renderChunks) {
        target.draw(chunk.aplhaLayerVerticies, activeStates);
    }
}