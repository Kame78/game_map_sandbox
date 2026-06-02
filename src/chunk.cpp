#include "chunk.hpp"
#include "map.hpp"

Chunk::Chunk(int chunkX, int chunkY, int chunkSize, int tileSize)
    : m_chunkX(chunkX), m_chunkY(chunkY), m_chunkSize(chunkSize), m_tileSize(tileSize),
      m_localGrid(chunkSize * chunkSize, TileType::Floor)
      {
        m_floorVerticies.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_wallVerticies.setPrimitiveType(sf::PrimitiveType::Triangles);
      }

      TileType Chunk::GetTile(int localX, int localY) const {
        return m_localGrid[localY * m_chunkSize + localX];
      }

      void Chunk::SetTile(int localX, int localY, TileType type) {
        m_localGrid[localY * m_chunkSize + localX] = type;
      }

      void Chunk::UpdateGeometry(int mapWidth, int mapHeight) {
        m_floorVerticies.clear();
        m_wallVerticies.clear();

        int chunkStartX = m_chunkX * m_chunkSize;
        int chunkStartY = m_chunkY * m_chunkSize;

        for(int y = 0; y < m_chunkSize; ++y) {
            for(int x = 0; x < m_chunkSize; ++x) {
                TileType type = GetTile(x, y);

                int globalTileX = chunkStartX + x;
                int globalTileY = chunkStartY + y;

                if (globalTileX >= mapWidth || globalTileY >= mapHeight) {
                    continue;
                }

                float screenX1 = static_cast<float>(globalTileX * m_tileSize);
                float screenY1 = static_cast<float>(globalTileY * m_tileSize);
                float screenX2 = screenX1 + m_tileSize;
                float screenY2 = screenY1 + m_tileSize;

                float texU1 = 0.f;
                float texV1 = 0.f;

                if (type == TileType::Wall) {
                    texU1 = 32.f;
                    texV1 = 288.f;
                } else {
                    int variation = (globalTileX * 7 + globalTileY * 13) % 3;
                    texU1 = static_cast<float>(variation * m_tileSize);
                    texV1 = 0.f;
                }

                float texU2 = texU1 + m_tileSize;
                float texV2 = texV1 + m_tileSize;

                sf::Vertex topLeft, topRight, bottomLeft, bottomRight;
                topLeft.position = {screenX1, screenY1};
                topRight.position = {screenX2, screenY1};
                bottomLeft.position = {screenX1, screenY2};
                bottomRight.position = {screenX2, screenY2};

                topLeft.texCoords = {texU1, texV1};
                topRight.texCoords = {texU2, texV1};
                bottomLeft.texCoords = {texU1, texV2};
                bottomRight.texCoords = {texU2, texV2};

                sf::VertexArray& targetLayer = (type == TileType::Wall) ? m_wallVerticies : m_floorVerticies;

                targetLayer.append(topLeft);
                targetLayer.append(topRight);
                targetLayer.append(bottomRight);

                targetLayer.append(topLeft);
                targetLayer.append(bottomRight);
                targetLayer.append(bottomLeft);
            }
        }
      }