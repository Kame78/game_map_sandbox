#include "map_renderer.hpp"
#include "tile_registry.hpp"
#include <iostream>

bool MapRenderer::ConnectsTo(TileRuntimeId currentId, TileRuntimeId neighborId) {
    if (currentId == neighborId) return true;

    const auto& currentProps = TileRegistry::Instance().GetProperties(currentId);
    const auto& neighborProps = TileRegistry::Instance().GetProperties(neighborId);

    return neighborProps.blendPriority > currentProps.blendPriority;
}

bool MapRenderer::Initialize() {
    m_textureCache.clear();
    for (TileRuntimeId id = 0; id < 256; ++id) {
        try {
            std::string path = TileRegistry::Instance().GetProperties(id).texturePath;
            if (path.empty()) continue;

            if(m_textureCache.find(path) == m_textureCache.end()) {
                sf::Texture tex;
                if(!tex.loadFromFile(path)) {
                    std::cerr << "[Renderer] Asset Error: Failed to compile texture from disk: " << path << "\n";
                    return false;
                }
                m_textureCache[path] = tex;
                std::cout << "[Renderer] Hydrated texture asset slot: " << path << "\n";
                }
            } catch(...) {
                break;
            }
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

    if (index < 0 || index >= static_cast<int>(m_renderChunks.size())) return;
    
    RenderChunk& rChunk = m_renderChunks[index];

    rChunk.baseLayers.clear();
    rChunk.alphaLayers.clear();

    int chunkSize = map.GetChunkSize();
    int fallbackTileSize = static_cast<float>(map.GetTileSize());
    int chunkStartX = cx * chunkSize;
    int chunkStartY = cy * chunkSize;

    auto GetSafeTile = [&] (int targetX, int targetY, TileRuntimeId fallbackId) -> TileRuntimeId {
        if( targetX < 0 || targetX >= map.GetWidth() || targetY < 0 || targetY >= map.GetHeight()) {
            return fallbackId;
        }
        return map.GetTile(targetX, targetY);
    };

    auto applyShadowColor = [](float intensityMultiplier, std::uint8_t alphaChannel) -> sf::Color {
        auto colorValue = static_cast<std::uint8_t>(255.0f * intensityMultiplier);
        return sf::Color(colorValue, colorValue, colorValue, alphaChannel);
    };

    for (int y = 0; y < chunkSize; ++y) {
        for (int x = 0; x < chunkSize; ++x) {
            int globalX = chunkStartX + x;
            int globalY = chunkStartY + y;

            if (globalX >=map.GetWidth() || globalY >= map.GetHeight()) continue;
            
            TileRuntimeId currentTileId = map.GetTile(globalX, globalY);
            const auto& props = TileRegistry::Instance().GetProperties(currentTileId);
            float currentSize = static_cast<float>(props.tileSize);

            float screenX1 = static_cast<float>(globalX * fallbackTileSize);
            float screenY1 = static_cast<float>(globalY * fallbackTileSize);
            float screenX2 = screenX1 + fallbackTileSize;
            float screenY2 = screenY1 + fallbackTileSize;

            TileRuntimeId lowestNeighborId = currentTileId;
            int lowestPriority = props.blendPriority;

            for (int ny = -1; ny <= 1; ++ny) {
                for (int nx = -1; nx <= 1; ++nx) {
                    TileRuntimeId nId = GetSafeTile(globalX + nx, globalY + ny, currentTileId);
                    int nPriority = TileRegistry::Instance().GetProperties(nId).blendPriority;
                    if (nPriority < lowestPriority) {
                        lowestPriority = nPriority;
                        lowestNeighborId = nId;
                    }
                }
            }

            //---------------------Ambient Occulsion
            float tlShadow = 1.0f; float trShadow = 1.0f;
            float blShadow = 1.0f; float brShadow = 1.0f;

            int currentPriority = props.blendPriority;

            //Gather the core heights of surrounding terrain blocks

            int pN = TileRegistry::Instance().GetProperties(GetSafeTile(globalX, globalY, currentTileId)).blendPriority;
            int pS = TileRegistry::Instance().GetProperties(GetSafeTile(globalX, globalY + 1, currentTileId)).blendPriority;
            int pW = TileRegistry::Instance().GetProperties(GetSafeTile(globalX - 1, globalY, currentTileId)).blendPriority;
            int pE = TileRegistry::Instance().GetProperties(GetSafeTile(globalX + 1, globalY, currentTileId)).blendPriority;

            int pNW = TileRegistry::Instance().GetProperties(GetSafeTile(globalX - 1, globalY - 1, currentTileId)).blendPriority;
            int pNE = TileRegistry::Instance().GetProperties(GetSafeTile(globalX + 1, globalY - 1, currentTileId)).blendPriority;
            int pSW = TileRegistry::Instance().GetProperties(GetSafeTile(globalX - 1, globalY + 1, currentTileId)).blendPriority;
            int pSE = TileRegistry::Instance().GetProperties(GetSafeTile(globalX + 1, globalY + 1, currentTileId)).blendPriority;

            // Apply direct orthogonal drop shadows (0.68 multiplier creates rich, distinct edge tracking)
            if (pN > currentPriority) { tlShadow *= 0.68; trShadow *= 0.68; }
            if (pN > currentPriority) { blShadow *= 0.68; brShadow *= 0.68; }
            if (pN > currentPriority) { tlShadow *= 0.68; blShadow *= 0.68; }
            if (pN > currentPriority) { trShadow *= 0.68; brShadow *= 0.68; }

            // Apply subtler ambient corner occlusion from diagonal elements (0.82 multiplier)
            if (pNW > currentPriority) tlShadow *= 0.82;
            if (pNE > currentPriority) trShadow *= 0.82;
            if (pSW > currentPriority) blShadow *= 0.82;
            if (pSE > currentPriority) brShadow *= 0.82;

            //Geometry build base layer

            const auto& baseProps = TileRegistry::Instance().GetProperties(lowestNeighborId);
            std::string baseKey = baseProps.texturePath;
            sf::VertexArray& baseVA = rChunk.baseLayers[baseKey];

            if (baseVA.getPrimitiveType() != sf::PrimitiveType::Triangles) {
                baseVA.setPrimitiveType(sf::PrimitiveType::Triangles);
            }

            GridLocation baseLoc = TileRegistry::Instance().GetAutotileLocation(baseProps.layoutProfile, 255);
            float baseU1 = static_cast<float>(baseLoc.col * baseProps.tileSize);
            float baseV1 = static_cast<float>(baseLoc.row * baseProps.tileSize);
            float baseU2 = baseU1 + static_cast<float>(baseProps.tileSize);
            float baseV2 = baseV1 + static_cast<float>(baseProps.tileSize);

            sf::Vertex b_topLeft, b_topRight, b_bottomLeft, b_bottomRight;
            b_topLeft.position = { screenX1, screenY1 }; b_topRight.position = { screenX2, screenY1 };
            b_bottomLeft.position = { screenX1, screenY2 }; b_bottomRight.position = { screenX2, screenY2 };
            b_topLeft.texCoords = { baseU1, baseV1 }; b_topRight.texCoords = { baseU2, baseV1 };
            b_bottomLeft.texCoords = { baseU1, baseV2 }; b_bottomRight.texCoords = { baseU2, baseV2 };

            std::uint8_t baseAlpha = (baseProps.internalName == "shallow_water") ? 128 : 255;
            b_topLeft.color     = applyShadowColor(tlShadow, baseAlpha);
            b_topRight.color    = applyShadowColor(trShadow, baseAlpha);
            b_bottomLeft.color  = applyShadowColor(blShadow, baseAlpha);
            b_bottomRight.color = applyShadowColor(brShadow, baseAlpha);

            baseVA.append(b_topLeft); baseVA.append(b_topRight); baseVA.append(b_bottomRight);
            baseVA.append(b_topLeft); baseVA.append(b_bottomRight); baseVA.append(b_bottomLeft);

            if( currentTileId == lowestNeighborId) continue;

            // Geometry Build for Alpha Overlay

            std::string fgKey = props.texturePath;
            if (fgKey.empty()) continue;
            sf::VertexArray& alphaVA = rChunk.alphaLayers[fgKey];

            if(alphaVA.getPrimitiveType() != sf::PrimitiveType::Triangles) {
                alphaVA.setPrimitiveType(sf::PrimitiveType::Triangles);
            }
            
            int bitmask = 0;
            if (ConnectsTo(currentTileId, map.GetTile(globalX - 1, globalY)))       bitmask |= 1;       //Left
            if (ConnectsTo(currentTileId, map.GetTile(globalX, globalY + 1)))       bitmask |= 2;       //Down
            if (ConnectsTo(currentTileId, map.GetTile(globalX + 1, globalY)))       bitmask |= 4;       //Right
            if (ConnectsTo(currentTileId, map.GetTile(globalX, globalY - 1)))       bitmask |= 8;       //Up
            if (ConnectsTo(currentTileId, map.GetTile(globalX - 1, globalY - 1)))   bitmask |= 16;    //North_West
            if (ConnectsTo(currentTileId, map.GetTile(globalX - 1, globalY + 1)))   bitmask |= 32;    //South_West
            if (ConnectsTo(currentTileId, map.GetTile(globalX + 1, globalY + 1)))   bitmask |= 64;    //South_East
            if (ConnectsTo(currentTileId, map.GetTile(globalX + 1, globalY - 1)))   bitmask |= 128;   //North_East

            if (!(bitmask & 8) || !(bitmask & 1)) bitmask &= ~16;
            if (!(bitmask & 2) || !(bitmask & 1)) bitmask &= ~32;
            if (!(bitmask & 2) || !(bitmask & 4)) bitmask &= ~64;
            if (!(bitmask & 8) || !(bitmask & 4)) bitmask &= ~128;

            GridLocation fgLoc = TileRegistry::Instance().GetAutotileLocation(props.layoutProfile, bitmask);
            float fgU1 = static_cast<float>(fgLoc.col * props.tileSize);
            float fgV1 = static_cast<float>(fgLoc.row * props.tileSize);
            float fgU2 = fgU1 + currentSize;
            float fgV2 = fgV1 + currentSize;

            sf::Vertex f_topLeft, f_topRight, f_bottomLeft, f_bottomRight;

            f_topLeft.position = { screenX1, screenY1 }; f_topRight.position = { screenX2, screenY1 };
            f_bottomLeft.position = { screenX1, screenY2 }; f_bottomRight.position = { screenX2, screenY2 };
            f_topLeft.texCoords = { fgU1, fgV1 }; f_topRight.texCoords = { fgU2, fgV1 };
            f_bottomLeft.texCoords = { fgU1, fgV2 }; f_bottomRight.texCoords = { fgU2, fgV2 };

            std::uint8_t fgAlpha = (props.internalName == "shallow_water") ? 128 :255;
            f_topLeft.color = applyShadowColor(tlShadow, fgAlpha);
            f_topRight.color = applyShadowColor(trShadow, fgAlpha);
            f_bottomLeft.color = applyShadowColor(blShadow, fgAlpha);
            f_bottomRight.color = applyShadowColor(brShadow, fgAlpha);

            alphaVA.append(f_topLeft); alphaVA.append(f_topRight); alphaVA.append(f_bottomRight);
            alphaVA.append(f_topLeft); alphaVA.append(f_bottomRight); alphaVA.append(f_bottomLeft);
        }
    }
}

void MapRenderer::Draw(sf::RenderTarget& target, const sf::RenderStates states) const {
    sf::RenderStates activeStates = states;

    for (const auto& chunk : m_renderChunks) {
        for(const auto& [path, vertexArray] : chunk.baseLayers) {
            if(vertexArray.getVertexCount() == 0) continue;
            auto it = m_textureCache.find(path);
            if(it != m_textureCache.end()) {
                activeStates.texture = &it->second;
                target.draw(vertexArray, activeStates);
            }
        }   
    }

    for (const auto& chunk : m_renderChunks) {
        for(const auto& [path, vertexArray] : chunk.alphaLayers) {
            if(vertexArray.getVertexCount() == 0) continue;
            auto it = m_textureCache.find(path);
            if(it != m_textureCache.end()) {
                activeStates.texture = &it->second;
                target.draw(vertexArray, activeStates);
            }
        }
    }
}